// System imports
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Project imports
#include "task.h"
#include "logger.h"
#include "double_linked_list.h"

// Unit test framework imports
#include "unit_test_infra.h"

#define TEST_EVENTS_PATH  "/tmp/test_events_unit.csv"
#define TEST_SUMMARY_PATH "/tmp/test_summary_unit.csv"

/* ── task_create fields ─────────────────────────────────────── */

TEST(test_task_create_fields) {
    task_t *t = task_create(7, 3, 100);
    ASSERT(t != NULL, "task_create should not return NULL");

    ASSERT(t->id         == 7,          "id should be 7");
    ASSERT(t->tickets    == 3,          "tickets should be 3");
    ASSERT(t->work_units == 100,        "work_units should be 100");
    ASSERT(t->first_dispatch == 0,      "first_dispatch should be 0 before any run");
    ASSERT(t->last_dispatch  == 0,      "last_dispatch should be 0 before any run");
    ASSERT(t->work_units_done == 0,     "work_units_done should start at 0");
    ASSERT(t->dispatches == 0,          "dispatches should start at 0");
    ASSERT(t->state == TASK_READY,      "initial state should be TASK_READY");

    task_destroy(t);
    return 0;
}

TEST(test_task_create_pi_state) {
    task_t *t = task_create(1, 1, 1);
    ASSERT(t != NULL, "task_create should not return NULL");

    ASSERT(t->pi.sum  == 2.0, "pi.sum should start at 2.0");
    ASSERT(t->pi.term == 1.0, "pi.term should start at 1.0");
    ASSERT(t->pi.j    == 0,   "pi.j should start at 0");

    task_destroy(t);
    return 0;
}

/* ── dispatch timestamps ────────────────────────────────────── */

TEST(test_first_dispatch_set_on_first_run) {
    task_t *t = task_create(1, 1, 50);
    ASSERT(t != NULL, "task_create should not return NULL");

    ASSERT(task_transition_to_running(t, 1) == true,
           "first transition to running should succeed");

    ASSERT(t->first_dispatch != 0, "first_dispatch should be set after first run");
    ASSERT(t->last_dispatch  != 0, "last_dispatch should be set after first run");
    ASSERT(t->dispatches == 1,     "dispatch count should be 1");

    task_destroy(t);
    return 0;
}

TEST(test_first_dispatch_stable_on_second_run) {
    task_t *t = task_create(2, 1, 50);
    ASSERT(t != NULL, "task_create should not return NULL");

    ASSERT(task_transition_to_running(t, 1) == true,
           "first transition to running should succeed");
    time_t saved_first = t->first_dispatch;

    ASSERT(task_transition_to_ready(t) == true,
           "transition back to ready should succeed");

    ASSERT(task_transition_to_running(t, 2) == true,
           "second transition to running should succeed");

    ASSERT(t->first_dispatch == saved_first,
           "first_dispatch should not change on second run");
    ASSERT(t->dispatches == 2,
           "dispatch count should be 2 after two runs");

    task_destroy(t);
    return 0;
}

/* ── logger_write_summary ───────────────────────────────────── */

static task_t *make_task_with_done(uint32_t id, uint32_t tickets,
                                   uint32_t work_units, uint32_t done)
{
    task_t *t = task_create(id, tickets, work_units);
    if (t) t->work_units_done = done;
    return t;
}

/* Builds a task list node-by-node without taking ownership of the
 * task_t pointers, so callers keep destroying them with task_destroy. */
static struct node *make_task_list(task_t **tasks, uint32_t count)
{
    struct node *head = NULL;
    for (uint32_t i = 0; i < count; i++) {
        dll_insert_node(&head, tasks[i], tasks[i]->id,
                         tasks[i]->tickets, tasks[i]->work_units);
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

TEST(test_logger_write_summary_creates_file) {
    ASSERT(logger_init_structure(TEST_EVENTS_PATH, TEST_SUMMARY_PATH) == true,
           "logger_init_structure should succeed");

    task_t *t1 = make_task_with_done(1, 3, 100, 10);
    task_t *t2 = make_task_with_done(2, 5, 200, 20);
    task_t *t3 = make_task_with_done(3, 2, 150, 30);
    ASSERT(t1 && t2 && t3, "all task_create calls should succeed");

    task_t *tasks[] = {t1, t2, t3};
    struct node *list_head = make_task_list(tasks, 3);
    ASSERT(logger_write_summary(list_head) == true,
           "logger_write_summary should return true");

    ASSERT(access(TEST_SUMMARY_PATH, F_OK) == 0,
           "summary file should exist after write");

    free_list_nodes(list_head);
    task_destroy(t1); task_destroy(t2); task_destroy(t3);
    return 0;
}

TEST(test_logger_write_summary_header) {
    ASSERT(logger_init_structure(TEST_EVENTS_PATH, TEST_SUMMARY_PATH) == true,
           "logger_init_structure should succeed");

    task_t *t1 = make_task_with_done(1, 1, 10, 5);
    ASSERT(t1 != NULL, "task_create should succeed");

    task_t *tasks[] = {t1};
    struct node *list_head = make_task_list(tasks, 1);
    ASSERT(logger_write_summary(list_head) == true,
           "logger_write_summary should return true");

    FILE *f = fopen(TEST_SUMMARY_PATH, "r");
    ASSERT(f != NULL, "should be able to open summary file for reading");

    char header[512];
    ASSERT(fgets(header, sizeof(header), f) != NULL,
           "should be able to read header line");
    fclose(f);

    ASSERT(strstr(header, "task_id")             != NULL, "header missing: task_id");
    ASSERT(strstr(header, "tickets")             != NULL, "header missing: tickets");
    ASSERT(strstr(header, "work_units_assigned") != NULL, "header missing: work_units_assigned");
    ASSERT(strstr(header, "work_units_completed")!= NULL, "header missing: work_units_completed");
    ASSERT(strstr(header, "dispatches")          != NULL, "header missing: dispatches");
    ASSERT(strstr(header, "first_dispatch")      != NULL, "header missing: first_dispatch");
    ASSERT(strstr(header, "last_dispatch")        != NULL, "header missing: last_dispatch");
    ASSERT(strstr(header, "pi_approx")           != NULL, "header missing: pi_approx");
    ASSERT(strstr(header, "observed_share")      != NULL, "header missing: observed_share");

    free_list_nodes(list_head);
    task_destroy(t1);
    return 0;
}

TEST(test_logger_write_summary_row_count) {
    ASSERT(logger_init_structure(TEST_EVENTS_PATH, TEST_SUMMARY_PATH) == true,
           "logger_init_structure should succeed");

    task_t *t1 = make_task_with_done(1, 3, 100, 10);
    task_t *t2 = make_task_with_done(2, 5, 200, 20);
    task_t *t3 = make_task_with_done(3, 2, 150, 30);
    ASSERT(t1 && t2 && t3, "all task_create calls should succeed");

    task_t *tasks[] = {t1, t2, t3};
    struct node *list_head = make_task_list(tasks, 3);
    ASSERT(logger_write_summary(list_head) == true,
           "logger_write_summary should return true");

    FILE *f = fopen(TEST_SUMMARY_PATH, "r");
    ASSERT(f != NULL, "should be able to open summary file for reading");

    int lines = 0;
    char buf[512];
    while (fgets(buf, sizeof(buf), f)) lines++;
    fclose(f);

    ASSERT(lines == 4, "expected 4 lines: 1 header + 3 task rows");

    free_list_nodes(list_head);
    task_destroy(t1); task_destroy(t2); task_destroy(t3);
    return 0;
}

TEST(test_logger_write_summary_observed_share_sums_to_one) {
    ASSERT(logger_init_structure(TEST_EVENTS_PATH, TEST_SUMMARY_PATH) == true,
           "logger_init_structure should succeed");

    task_t *t1 = make_task_with_done(1, 3, 100, 10);
    task_t *t2 = make_task_with_done(2, 5, 200, 20);
    task_t *t3 = make_task_with_done(3, 2, 150, 30);
    ASSERT(t1 && t2 && t3, "all task_create calls should succeed");

    task_t *tasks[] = {t1, t2, t3};
    struct node *list_head = make_task_list(tasks, 3);
    ASSERT(logger_write_summary(list_head) == true,
           "logger_write_summary should return true");

    FILE *f = fopen(TEST_SUMMARY_PATH, "r");
    ASSERT(f != NULL, "should be able to open summary file for reading");

    /* skip header */
    char buf[512];
    ASSERT(fgets(buf, sizeof(buf), f) != NULL,
           "should be able to read header line");

    double share_sum = 0.0;
    while (fgets(buf, sizeof(buf), f)) {
        /* observed_share is the last field */
        char *last_comma = strrchr(buf, ',');
        ASSERT(last_comma != NULL, "row should contain a comma");
        share_sum += atof(last_comma + 1);
    }
    fclose(f);

    ASSERT(fabs(share_sum - 1.0) < 0.0001,
           "observed_share values should sum to 1.0");

    free_list_nodes(list_head);
    task_destroy(t1); task_destroy(t2); task_destroy(t3);
    return 0;
}

/* ── main ───────────────────────────────────────────────────── */

int main(void) {
    printf("=== Summary / Task fields — Unit tests ===\n\n");

    RUN(test_task_create_fields);
    RUN(test_task_create_pi_state);
    RUN(test_first_dispatch_set_on_first_run);
    RUN(test_first_dispatch_stable_on_second_run);
    RUN(test_logger_write_summary_creates_file);
    RUN(test_logger_write_summary_header);
    RUN(test_logger_write_summary_row_count);
    RUN(test_logger_write_summary_observed_share_sums_to_one);

    return 0;
}