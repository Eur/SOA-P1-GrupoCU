// System imports
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Unit test framework imports
#include "unit_test_infra.h"
#include "experiment_stats.h"

#define QUANTUM_ARGS       "--mode quantum --quantum 1000"

/* ── Case 3: equal shares ───────────────────────────────────── */

TEST(test_case3_equal_shares)
{
    task_stats_t stats[MAX_TASKS];
    int tasks = run_experiment("case3", EQUAL_SHARES_INPUT, QUANTUM_ARGS, stats);
    ASSERT(tasks == 5, "case 3 should run 5 tasks over all seeds");

    double mean_error = print_stats("Case 3: equal shares", "30 seeds, 10000 dispatches, quantum 1000", stats, tasks, MAX_MEAN_ABS_ERROR);

    for (int i = 0; i < tasks; i++) {
        ASSERT(fabs(stats[i].mean_share - 0.20) <= MAX_MEAN_ABS_ERROR,
               "case 3: mean share of every task should be ~0.20");
    }
    ASSERT(mean_error <= MAX_MEAN_ABS_ERROR,
           "case 3: mean absolute error should be <= 0.02");
    return 0;
}

/* ── Case 4: proportionality ────────────────────────────────── */

TEST(test_case4_proportionality)
{
    task_stats_t stats[MAX_TASKS];
    int tasks = run_experiment("case4", PROPORTIONAL_INPUT, QUANTUM_ARGS, stats);
    ASSERT(tasks == 5, "case 4 should run 5 tasks over all seeds");

    double mean_error = print_stats("Case 4: proportionality", "30 seeds, 10000 dispatches, quantum 1000", stats, tasks, MAX_MEAN_ABS_ERROR);

    for (int i = 0; i < tasks; i++) {
        ASSERT(stats[i].abs_error <= MAX_MEAN_ABS_ERROR,
               "case 4: abs error of every task should be <= 0.02");
    }
    ASSERT(mean_error <= MAX_MEAN_ABS_ERROR,
           "case 4: mean absolute error should be <= 0.02");
    return 0;
}

/* ── main ───────────────────────────────────────────────────── */

int main(void)
{
    printf("=== Experiments (cases 3 and 4) — Unit tests ===\n");

    RUN(test_case3_equal_shares);
    RUN(test_case4_proportionality);

    printf("\nSummary: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
