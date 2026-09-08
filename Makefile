CC := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude -pthread


TARGET := lottery_scheduler
SOURCES := src/main.c src/double_linked_list.c src/parser.c src/rng.c src/scheduler.c src/task.c src/logger.c

OBJECTS := $(SOURCES:.c=.o)

.PHONY: all test clean
.PHONY: asan tsan

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)



ASAN_TARGET := lottery_scheduler_asan

ASAN_CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -g
ASAN_OBJECTS := $(SOURCES:.c=.asan.o)

asan: $(ASAN_TARGET)
	export ASAN_OPTIONS=log_path=./asan_report:detect_leaks=1; ./$(ASAN_TARGET)

$(ASAN_TARGET): $(ASAN_OBJECTS)
	$(CC) $(ASAN_CFLAGS) $(ASAN_OBJECTS) -o $@

src/%.asan.o: src/%.c
	$(CC) $(ASAN_CFLAGS) -c $< -o $@


TSAN_TARGET := lottery_scheduler_tsan

TSAN_CFLAGS := $(CFLAGS) -fsanitize=thread -fno-omit-frame-pointer -g
TSAN_OBJECTS := $(SOURCES:.c=.tsan.o)

tsan: $(TSAN_TARGET)
	TSAN_OPTIONS=log_path=./tsan_report:halt_on_error=1 ./$(TSAN_TARGET)

$(TSAN_TARGET): $(TSAN_OBJECTS)
	$(CC) $(TSAN_CFLAGS) $(TSAN_OBJECTS) -o $@

src/%.tsan.o: src/%.c
	$(CC) $(TSAN_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(ASAN_OBJECTS) $(TSAN_OBJECTS) \
		$(TARGET) $(ASAN_TARGET) $(TSAN_TARGET)
