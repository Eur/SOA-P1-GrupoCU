CC     := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -I../../include -Iinclude -pthread

TEST_PARSER  := /tmp/test_parser	
TEST_SRCS := test_parser.c ../../src/double_linked_list.c ../../src/parser.c ../../src/task.c




.PHONY: all clean

test_parser: $(TEST_PARSER)
	$(TEST_PARSER)

$(TEST_PARSER): $(TEST_SRCS)
	$(CC) $(CFLAGS) $^ -o $@

clean_test_parser:
	rm -f $(TEST_PARSER)
