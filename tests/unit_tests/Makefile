CC     := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -I../../include -Iinclude -pthread

TEST_PARSER  := /tmp/test_parser	
TEST_SRCS := test_parser.c ../../src/double_linked_list.c ../../src/parser.c ../../src/task.c

TEST_DLL := /tmp/test_dll
TEST_DLL_SRCS := test_dll.c ../../src/double_linked_list.c

TEST_RNG := /tmp/test_rng
TEST_RNG_SRCS := test_rng.c ../../src/rng.c

TEST_TASK := /tmp/test_task
TEST_TASK_SRCS := test_task.c ../../src/task.c

.PHONY: all clean
all: test_parser test_dll test_rng test_task

clean: clean_test_parser clean_test_dll clean_test_rng clean_test_task

## Parser unit tests
test_parser: $(TEST_PARSER)
	$(TEST_PARSER)
$(TEST_PARSER): $(TEST_SRCS)
	$(CC) $(CFLAGS) $^ -o $@
clean_test_parser:
	rm -f $(TEST_PARSER)

## Double Linked List unit tests
test_dll: $(TEST_DLL)
	$(TEST_DLL)
$(TEST_DLL): $(TEST_DLL_SRCS)
	$(CC) $(CFLAGS) $^ -o $@
clean_test_dll:
	rm -f $(TEST_DLL)

## RNG unit tests
test_rng: $(TEST_RNG)
	$(TEST_RNG)
$(TEST_RNG): $(TEST_RNG_SRCS)
	$(CC) $(CFLAGS) $^ -o $@
clean_test_rng:
	rm -f $(TEST_RNG)

## Task unit tests
test_task: $(TEST_TASK)
	$(TEST_TASK)
$(TEST_TASK): $(TEST_TASK_SRCS)
	$(CC) $(CFLAGS) $^ -o $@
clean_test_task:
	rm -f $(TEST_TASK)
