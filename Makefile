CC := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -Iinclude -pthread

TARGET := lotery_scheduler
SOURCES := src/main.c src/double_linked_list.c src/parser.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all test clean
.PHONY: asan

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

ASAN_TARGET := lotery_scheduler_asan
ASAN_CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer -g
ASAN_OBJECTS := $(SOURCES:.c=.asan.o)

asan: $(ASAN_TARGET)
	export ASAN_OPTIONS=log_path=./asan_report; ./$(ASAN_TARGET)

$(ASAN_TARGET): $(ASAN_OBJECTS)
	$(CC) $(ASAN_CFLAGS) $(ASAN_OBJECTS) -o $@

src/%.asan.o: src/%.c
	$(CC) $(ASAN_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(ASAN_OBJECTS) $(TARGET) $(ASAN_TARGET)
