CC := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude -pthread
LDLIBS := -lm


TARGET := lottery_scheduler
SOURCES := src/main.c src/double_linked_list.c src/parser.c src/rng.c src/scheduler.c src/task.c src/logger.c

OBJECTS := $(SOURCES:.c=.o)

.PHONY: all test clean
.PHONY: asan asan_cooperative asan_quantum tsan tsan_cooperative tsan_quantum

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
	$(CC) $(ASAN_CFLAGS) $(ASAN_OBJECTS) -o $@

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
	$(CC) $(TSAN_CFLAGS) $(TSAN_OBJECTS) -o $@

src/%.tsan.o: src/%.c
	$(CC) $(TSAN_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(ASAN_OBJECTS) $(TSAN_OBJECTS) \
		$(TARGET) $(ASAN_TARGET) $(TSAN_TARGET)
