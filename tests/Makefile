CC     := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -I../include -pthread

TEST_BIN  := /tmp/test_parser	
TEST_SRCS := test_parser.c ../src/double_linked_list.c ../src/parser.c ../src/task.c

.PHONY: all clean

all: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TEST_BIN)
