CC     := gcc
CFLAGS := -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror -I../../include -Iinclude -pthread

TEST_PARSER  := /tmp/test_parser	
TEST_SRCS := test_parser.c ../../src/double_linked_list.c ../../src/parser.c ../../src/task.c

TEST_DLL := /tmp/test_dll
TEST_DLL_SRCS := test_dll.c ../../src/double_linked_list.c

TEST_RNG := /tmp/test_rng
TEST_RNG_SRCS := test_rng.c ../../src/rng.c

TEST_SUMMARY := /tmp/test_summary
TEST_SUMMARY_SRCS := test_summary.c \
                     ../../src/task.c \
                     ../../src/parser.c \
                     ../../src/double_linked_list.c \
                     ../../src/logger.c

TEST_COOPERATIVE := /tmp/test_cooperative
TEST_COOPERATIVE_SRCS := test_cooperative.c \
                         ../../src/task.c \
                         ../../src/parser.c \
                         ../../src/double_linked_list.c

TEST_QUANTUM := /tmp/test_quantum
TEST_QUANTUM_SRCS := test_quantum.c \
                     ../../src/scheduler.c \
                     ../../src/task.c \
                     ../../src/parser.c \
                     ../../src/double_linked_list.c \
                     ../../src/rng.c \
                     ../../src/logger.c

all: test_parser test_dll test_rng test_summary test_cooperative test_quantum test_experiments test_mode_comparison

clean: clean_test_parser clean_test_dll clean_test_rng clean_test_summary clean_test_cooperative clean_test_quantum clean_test_experiments clean_test_mode_comparison
.PHONY: all clean

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


test_summary: $(TEST_SUMMARY)
	$(TEST_SUMMARY)
$(TEST_SUMMARY): $(TEST_SUMMARY_SRCS) include/experiment_stats.h
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -lm
clean_test_summary:
	rm -f $(TEST_SUMMARY)

	
test_cooperative: $(TEST_COOPERATIVE)
	$(MAKE) -C ../.. all
	$(TEST_COOPERATIVE)
$(TEST_COOPERATIVE): $(TEST_COOPERATIVE_SRCS) include/experiment_stats.h
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -lm
clean_test_cooperative:
	rm -f $(TEST_COOPERATIVE)

## Quantum mode / ticket compensation unit tests
test_quantum: $(TEST_QUANTUM)
	$(MAKE) -C ../.. all
	$(TEST_QUANTUM)
$(TEST_QUANTUM): $(TEST_QUANTUM_SRCS) include/experiment_stats.h
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -lm
clean_test_quantum:
	rm -f $(TEST_QUANTUM)

## Experiments (cases 3 and 4): runs the scheduler binary over 30 seeds
TEST_EXPERIMENTS := /tmp/test_experiments_bin
TEST_EXPERIMENTS_SRCS := test_experiments.c

test_experiments: $(TEST_EXPERIMENTS)
	$(MAKE) -C ../.. all
	$(TEST_EXPERIMENTS)
$(TEST_EXPERIMENTS): $(TEST_EXPERIMENTS_SRCS) include/experiment_stats.h
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ -lm
clean_test_experiments:
	rm -f $(TEST_EXPERIMENTS)

## Mode comparison (case 6): cooperative vs quantum, same input and seed
TEST_MODE_COMPARISON := /tmp/test_mode_comparison_bin
TEST_MODE_COMPARISON_SRCS := test_mode_comparison.c

test_mode_comparison: $(TEST_MODE_COMPARISON)
	$(MAKE) -C ../.. all
	$(TEST_MODE_COMPARISON)
$(TEST_MODE_COMPARISON): $(TEST_MODE_COMPARISON_SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -lm
clean_test_mode_comparison:
	rm -f $(TEST_MODE_COMPARISON)
