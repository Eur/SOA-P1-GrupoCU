// System imports
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Local project imports
#include "double_linked_list.h"
#include "task.h"
#include "scheduler.h"

// Unit test framework imports
#include "unit_test_infra.h"
#include "experiment_stats.h"

#define QUANTUM_ARGS "--mode quantum --quantum 500"


/* ── task_apply_compensation ──────────────────────────────────────────── */

TEST(test_compensation_disabled_resets_to_tickets)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = 10;
    task.yield_fraction = 0.5;
    task.effective_tickets = 999; /* stale value from a previous dispatch */

    task_apply_compensation(&task, false);

    ASSERT(task.effective_tickets == 10,
           "compensation disabled: effective_tickets should reset to tickets");
    return 0;
}

TEST(test_compensation_full_yield_no_boost)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = 10;
    task.yield_fraction = 1.0; /* ran to completion, nothing to compensate */
    task.effective_tickets = 999;

    task_apply_compensation(&task, true);

    ASSERT(task.effective_tickets == 10,
           "yield_fraction == 1.0: effective_tickets should equal tickets");
    return 0;
}

TEST(test_compensation_partial_yield_boosts_tickets)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = 10;
    task.yield_fraction = 0.5; /* only used half its quantum before yielding */

    task_apply_compensation(&task, true);

    /* round(10 / 0.5) = round(20.0) = 20 */
    ASSERT(task.effective_tickets == 20,
           "yield_fraction 0.5: effective_tickets should be tickets / yield_fraction");
    return 0;
}

TEST(test_compensation_rounds_to_nearest)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = 10;
    task.yield_fraction = 0.3; /* 10 / 0.3 = 33.33... -> rounds to 33 */

    task_apply_compensation(&task, true);

    ASSERT(task.effective_tickets == 33,
           "yield_fraction 0.3: effective_tickets should round to nearest integer");
    return 0;
}

TEST(test_compensation_clamps_to_uint32_max)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = UINT32_MAX;
    task.yield_fraction = 0.01; /* would overflow uint32_t without clamping */

    task_apply_compensation(&task, true);

    ASSERT(task.effective_tickets == UINT32_MAX,
           "an overflowing compensation should clamp to UINT32_MAX");
    return 0;
}

TEST(test_compensation_reapplied_each_dispatch)
{
    /* Simulates two consecutive dispatches of the same task: a boosted
     * effective_tickets from a previous partial run must not leak into
     * a dispatch where the task then runs to completion. */
    task_t task;
    memset(&task, 0, sizeof(task));
    task.tickets = 10;

    task.yield_fraction = 0.5;
    task_apply_compensation(&task, true);
    ASSERT(task.effective_tickets == 20, "first dispatch should boost to 20");

    task.yield_fraction = 1.0;
    task_apply_compensation(&task, true);
    ASSERT(task.effective_tickets == 10,
           "second dispatch running to completion should reset to tickets");
    return 0;
}


/* ── scheduler_configure_quantum ───────────────────────────────────────── */

static struct node *make_task_list(task_t **tasks, uint32_t count)
{
    struct node *head = NULL;
    dll_init_list(&head);
    for (uint32_t i = 0; i < count; i++) {
        bool inserted = dll_insert_node(&head, tasks[i], tasks[i]->id,
                                         tasks[i]->tickets, tasks[i]->work_units);
        if (!inserted) {
            fprintf(stderr, "  make_task_list: dll_insert_node failed\n");
        }
    }
    return head;
}

/* Frees only the list nodes, leaving the task_t data untouched
 * (the caller destroys those separately with task_destroy). */
static void free_list_nodes(struct node *head)
{
    while (head != NULL) {
        struct node *next = head->next;
        free(head);
        head = next;
    }
}

TEST(test_scheduler_configure_quantum_sets_slice_on_every_task)
{
    task_t *tasks[3];
    tasks[0] = task_create(0, 1, 10, 1.0);
    tasks[1] = task_create(1, 1, 10, 1.0);
    tasks[2] = task_create(2, 1, 10, 1.0);

    struct node *head = make_task_list(tasks, 3);

    scheduler_configure_quantum(head, 5);

    ASSERT(tasks[0]->slice_size == 5, "task 0 slice_size should be the quantum");
    ASSERT(tasks[1]->slice_size == 5, "task 1 slice_size should be the quantum");
    ASSERT(tasks[2]->slice_size == 5, "task 2 slice_size should be the quantum");

    free_list_nodes(head);
    for (uint32_t i = 0; i < 3; i++) {
        task_destroy(tasks[i]);
    }
    return 0;
}

TEST(test_scheduler_configure_quantum_zero_clamps_to_one)
{
    task_t *task = task_create(0, 1, 10, 1.0);
    struct node *head = make_task_list(&task, 1);

    scheduler_configure_quantum(head, 0);

    ASSERT(task->slice_size == 1, "quantum 0 should clamp slice_size to 1");

    free_list_nodes(head);
    task_destroy(task);
    return 0;
}


/* ── share statistics over 30 seeds ───────────────────────────────────── */

TEST(test_quantum_shares_are_equal_with_equal_tickets)
{
    task_stats_t stats[MAX_TASKS];
    int tasks = run_experiment("quantum_case3", EQUAL_SHARES_INPUT,
                               QUANTUM_ARGS, stats);
    ASSERT(tasks == 5, "quantum run should report 5 tasks over all seeds");

    double mean_error = print_stats("Quantum: equal shares",
                                    "30 seeds, 10000 dispatches, quantum 500",
                                    stats, tasks, MAX_MEAN_ABS_ERROR);

    for (int i = 0; i < tasks; i++) {
        ASSERT(stats[i].abs_error <= MAX_MEAN_ABS_ERROR,
               "quantum: abs error of every task should be <= 0.02");
    }
    ASSERT(mean_error <= MAX_MEAN_ABS_ERROR,
           "quantum: mean absolute error should be <= 0.02");
    return 0;
}


int main(void)
{
    printf("=== Quantum Mode / Compensation — Unit tests ===\n\n");

    RUN(test_compensation_disabled_resets_to_tickets);
    RUN(test_compensation_full_yield_no_boost);
    RUN(test_compensation_partial_yield_boosts_tickets);
    RUN(test_compensation_rounds_to_nearest);
    RUN(test_compensation_clamps_to_uint32_max);
    RUN(test_compensation_reapplied_each_dispatch);

    RUN(test_scheduler_configure_quantum_sets_slice_on_every_task);
    RUN(test_scheduler_configure_quantum_zero_clamps_to_one);
    RUN(test_quantum_shares_are_equal_with_equal_tickets);

    printf("\nSummary: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
