CC := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude -pthread
LDLIBS := -lm


TARGET := lottery_scheduler
SOURCES := src/main.c src/double_linked_list.c src/parser.c src/rng.c src/scheduler.c src/task.c src/logger.c

OBJECTS := $(SOURCES:.c=.o)

.PHONY: all test clean
.PHONY: asan asan_cooperative asan_quantum tsan tsan_cooperative tsan_quantum \
	ubsan ubsan_slice ubsan_quantum

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@set -eu; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	printf '\033[38;5;208m+ 1. Starting validation tests across invalid input CSV files +\033[0m\n'; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	for input_file in tests/invalid_input_files/*.csv; do \
		printf '\033[33m----------Begin testing for input file: %s----------\033[0m\n' "$$input_file"; \
		if ./$(TARGET) --input "$$input_file" --mode quantum --quantum 1 --seed 2026; then \
			exit_code=0; \
		else \
			exit_code=$$?; \
		fi; \
		if [ "$$exit_code" -eq 0 ]; then \
			printf '\033[31mFAIL: %s was accepted\033[0m\n' "$$input_file"; \
			exit 1; \
		else \
			printf '\033[32mPASS: %s was rejected\033[0m\n' "$$input_file"; \
		fi; \
	done \

	@set -eu; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	printf '\033[38;5;208m+ 2. Starting reproducibility validation                      +\033[0m\n'; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	reproducibility_input=tests/reproducibility_input_files/reproducibility_input_file.csv; \
	first_log=results/reproducibility_run_1.log; \
	second_log=results/reproducibility_run_2.log; \
	./$(TARGET) --input "$$reproducibility_input" --mode quantum --quantum 10 --seed 2026 --log "$$first_log"; \
	./$(TARGET) --input "$$reproducibility_input" --mode quantum --quantum 10 --seed 2026 --log "$$second_log"; \
	if diff -u "$$first_log" "$$second_log"; then \
		printf '\033[32mPASS: reproducibility logs are identical\033[0m\n'; \
	else \
		printf '\033[31mFAIL: reproducibility logs differ\033[0m\n'; \
		exit 1; \
	fi \

	@set -eu; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	printf '\033[38;5;208m+ 3. Starting stress validation                               +\033[0m\n'; \
	printf '\033[38;5;208m+-------------------------------------------------------------+\033[0m\n'; \
	run_stress_test() { \
		sanitizer=$$1; scenario=$$2; seed=$$3; \
		case "$$scenario" in \
			quantum) input=tests/stress_input_files/stress_input_quantum.csv; mode=quantum; parameter=--quantum; value=1000; compensation=0 ;; \
			quantum_compensation) input=tests/stress_input_files/stress_input_quantum_compensation.csv; mode=quantum; parameter=--quantum; value=1000; compensation=1 ;; \
			slice) input=tests/stress_input_files/stress_input_slice.csv; mode=cooperative; parameter=--slice-percent; value=10; compensation=0 ;; \
			slice_compensation) input=tests/stress_input_files/stress_input_slice_compensation.csv; mode=cooperative; parameter=--slice-percent; value=10; compensation=1 ;; \
			*) printf '\033[31mFAIL: unknown stress scenario %s\033[0m\n' "$$scenario"; return 1 ;; \
		esac; \
		case "$$sanitizer" in \
			asan) binary=./$(ASAN_TARGET); sanitizer_options=ASAN_OPTIONS=log_path=./asan_report:halt_on_error=1:detect_leaks=1 ;; \
			tsan) binary=./$(TSAN_TARGET); sanitizer_options=TSAN_OPTIONS=log_path=./at:san_reporthalt_on_error=1 ;; \
			ubsan) binary=./$(UBSAN_TARGET); sanitizer_options=UBSAN_OPTIONS=log_path=./ubsan_report:halt_on_error=1:print_stacktrace=1 ;; \
			*) printf '\033[31mFAIL: unknown sanitizer %s\033[0m\n' "$$sanitizer"; return 1 ;; \
		esac; \
		log=results/stress_$${sanitizer}_$${scenario}_seed_$${seed}.log; \
		printf '\033[38;5;208mTesting 25 tasks in %s mode with %s, seed %s, compensation %s\033[0m\n' "$$mode" "$$sanitizer" "$$seed" "$$([ "$$compensation" -eq 1 ] && printf activated || printf disabled)"; \
		set -- --input "$$input" --mode "$$mode" "$$parameter" "$$value" --seed "$$seed" --log "$$log"; \
		if [ "$$compensation" -eq 1 ]; then set -- "$$@" --compensation; fi; \
		if env "$$sanitizer_options" "$$binary" "$$@"; then \
			printf '\033[32mPASS: %s %s seed %s\033[0m\n' "$$sanitizer" "$$scenario" "$$seed"; \
		else \
			printf '\033[31mFAIL: %s %s seed %s\033[0m\n' "$$sanitizer" "$$scenario" "$$seed"; \
			return 1; \
		fi; \
	}; \
	for sanitizer in asan tsan ubsan; do \
		for scenario in quantum quantum_compensation slice slice_compensation; do \
			for seed in 2026 4242; do \
				run_stress_test "$$sanitizer" "$$scenario" "$$seed"; \
			done; \
		done; \
	done

	
	

ASAN_TARGET := lottery_scheduler_asan

ASAN_CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -g
ASAN_OBJECTS := $(SOURCES:.c=.asan.o)

asan: $(ASAN_TARGET)
	export ASAN_OPTIONS=log_path=./asan_report:detect_leaks=1; ./$(ASAN_TARGET)

asan_cooperative: $(ASAN_TARGET)
	export ASAN_OPTIONS=log_path=./asan_report:detect_leaks=1; ./$(ASAN_TARGET) --input tests/base.csv --mode cooperative --slice-percent 10 --seed 2026 --summary results/base_summary.csv

asan_quantum: $(ASAN_TARGET)
	export ASAN_OPTIONS=log_path=./asan_report:detect_leaks=1; ./$(ASAN_TARGET) --input tests/base.csv --mode quantum --quantum 1000 --seed 2026 --log results/base_events.csv --summary results/base_summary.csv

$(ASAN_TARGET): $(ASAN_OBJECTS)
	$(CC) $(ASAN_CFLAGS) $(ASAN_OBJECTS) $(LDLIBS) -o $@

src/%.asan.o: src/%.c
	$(CC) $(ASAN_CFLAGS) -c $< -o $@


TSAN_TARGET := lottery_scheduler_tsan

TSAN_CFLAGS := $(CFLAGS) -fsanitize=thread -fno-omit-frame-pointer -g
TSAN_OBJECTS := $(SOURCES:.c=.tsan.o)

tsan: $(TSAN_TARGET)
	TSAN_OPTIONS=log_path=./tsan_report:halt_on_error=1 ./$(TSAN_TARGET)

tsan_cooperative: $(TSAN_TARGET)
	TSAN_OPTIONS=log_path=./tsan_report:halt_on_error=1 ./$(TSAN_TARGET) --input tests/base.csv --mode cooperative --slice-percent 10 --seed 2026 --summary results/base_summary.csv

tsan_quantum: $(TSAN_TARGET)
	TSAN_OPTIONS=log_path=./tsan_report:halt_on_error=1 ./$(TSAN_TARGET) --input tests/base.csv --mode quantum --quantum 1000 --seed 2026 --log results/base_events.csv --summary results/base_summary.csv

$(TSAN_TARGET): $(TSAN_OBJECTS)
	$(CC) $(TSAN_CFLAGS) $(TSAN_OBJECTS) $(LDLIBS) -o $@

src/%.tsan.o: src/%.c
	$(CC) $(TSAN_CFLAGS) -c $< -o $@


UBSAN_TARGET := lottery_scheduler_ubsan

UBSAN_CFLAGS := $(CFLAGS) -fsanitize=undefined -fno-omit-frame-pointer -g
UBSAN_OBJECTS := $(SOURCES:.c=.ubsan.o)

ubsan: $(UBSAN_TARGET)
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./$(UBSAN_TARGET)

ubsan_slice: $(UBSAN_TARGET)
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./$(UBSAN_TARGET) --input tests/base.csv --mode cooperative --slice-percent 10 --seed 2026 --summary results/base_summary.csv

ubsan_quantum: $(UBSAN_TARGET)
	UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./$(UBSAN_TARGET) --input tests/base.csv --mode quantum --quantum 1000 --seed 2026 --log results/base_events.csv --summary results/base_summary.csv

$(UBSAN_TARGET): $(UBSAN_OBJECTS)
	$(CC) $(UBSAN_CFLAGS) $(UBSAN_OBJECTS) $(LDLIBS) -o $@

src/%.ubsan.o: src/%.c
	$(CC) $(UBSAN_CFLAGS) -c $< -o $@

test: $(ASAN_TARGET) $(TSAN_TARGET) $(UBSAN_TARGET)

clean:
	rm -f $(OBJECTS) $(ASAN_OBJECTS) $(TSAN_OBJECTS) $(UBSAN_OBJECTS) \
		$(TARGET) $(ASAN_TARGET) $(TSAN_TARGET) $(UBSAN_TARGET)
