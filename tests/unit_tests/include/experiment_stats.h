/* ── Share statistics helpers ───────────────────────────────── */
/* Runs the scheduler binary once per seed and aggregates the
 * observed_share of every task, so unit tests can print the same
 * target / mean / std_dev / abs_error table. */

#ifndef EXPERIMENT_STATS_H
#define EXPERIMENT_STATS_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SCHEDULER_BINARY   "../../lottery_scheduler"
#define EQUAL_SHARES_INPUT "../equal_shares_input_files/equal_shares_input_file.csv"
#define PROPORTIONAL_INPUT "../proportionality_input_files/proportionality_input_file.csv"
#define OUTPUT_ROOT        "/tmp/test_experiments"

#define SEEDS_COUNT        30
#define DISPATCHES         10000
#define MAX_TASKS          25
#define MAX_MEAN_ABS_ERROR 0.02

typedef struct {
    uint32_t task_id;
    uint32_t tickets;
    double   target_share;
    double   mean_share;
    double   std_dev;
    double   abs_error;
} task_stats_t;

/* Reads a summary CSV and stores task_id / tickets / observed_share per row.
 * Returns the number of task rows, or -1 on failure. */
static inline int read_summary(const char *path, uint32_t *ids,
                               uint32_t *tickets, double *shares)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) return -1;

    char line[512];
    if (fgets(line, sizeof(line), f) == NULL) { fclose(f); return -1; } /* header */

    int rows = 0;
    while (rows < MAX_TASKS && fgets(line, sizeof(line), f) != NULL) {
        char *field = strtok(line, ",");
        for (int col = 0; field != NULL; col++, field = strtok(NULL, ",")) {
            if (col == 0) ids[rows]     = (uint32_t)strtoul(field, NULL, 10);
            if (col == 1) tickets[rows] = (uint32_t)strtoul(field, NULL, 10);
            if (col == 8) shares[rows]  = atof(field);
        }
        rows++;
    }
    fclose(f);
    return rows;
}

/* Fills target / mean / std_dev / abs_error from per-task sums. */
static inline void finish_stats(task_stats_t *stats, int tasks,
                                const double *sum, const double *sum_sq,
                                int samples)
{
    uint32_t total_tickets = 0;
    for (int i = 0; i < tasks; i++) total_tickets += stats[i].tickets;

    for (int i = 0; i < tasks; i++) {
        double mean = sum[i] / samples;
        double var  = sum_sq[i] / samples - mean * mean;
        stats[i].target_share = (double)stats[i].tickets / total_tickets;
        stats[i].mean_share   = mean;
        stats[i].std_dev      = sqrt(var > 0.0 ? var : 0.0);
        stats[i].abs_error    = fabs(mean - stats[i].target_share);
    }
}

/* Runs the scheduler once per seed with `mode_args` (for example
 * "--mode quantum --quantum 1000") and aggregates observed_share per task.
 * Returns the number of tasks, or -1 on failure. */
static inline int run_experiment(const char *case_name, const char *input,
                                 const char *mode_args, task_stats_t *stats)
{
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/%s", OUTPUT_ROOT, case_name);
    if (system(cmd) != 0) return -1;

    double sum[MAX_TASKS]    = {0};
    double sum_sq[MAX_TASKS] = {0};
    int    tasks             = 0;

    for (int seed = 1; seed <= SEEDS_COUNT; seed++) {
        char summary[256];
        snprintf(summary, sizeof(summary), "%s/%s/summary_seed_%d.csv",
                 OUTPUT_ROOT, case_name, seed);
        snprintf(cmd, sizeof(cmd),
                 "%s --input %s %s --seed %d --max-dispatches %d "
                 "--summary %s > /dev/null",
                 SCHEDULER_BINARY, input, mode_args, seed, DISPATCHES, summary);
        if (system(cmd) != 0) return -1;

        uint32_t ids[MAX_TASKS], tickets[MAX_TASKS];
        double   shares[MAX_TASKS];
        int rows = read_summary(summary, ids, tickets, shares);
        if (rows <= 0) return -1;

        if (seed == 1) {
            tasks = rows;
            for (int i = 0; i < rows; i++) {
                stats[i].task_id = ids[i];
                stats[i].tickets = tickets[i];
            }
        } else if (rows != tasks) {
            return -1;
        }

        for (int i = 0; i < rows; i++) {
            sum[i]    += shares[i];
            sum_sq[i] += shares[i] * shares[i];
        }
    }

    finish_stats(stats, tasks, sum, sum_sq, SEEDS_COUNT);
    return tasks;
}

/* Prints the table and returns the mean absolute error across tasks.
 * A negative `limit` omits the limit from the output. */
static inline double print_stats(const char *title, const char *detail,
                                 const task_stats_t *stats, int tasks,
                                 double limit)
{
    printf("\n  %s (%s)\n", title, detail);
    printf("  %-6s %-8s %-10s %-10s %-10s %-10s\n",
           "task", "tickets", "target", "mean", "std_dev", "abs_error");

    double error_sum = 0.0;
    for (int i = 0; i < tasks; i++) {
        printf("  %-6u %-8u %-10.4f %-10.4f %-10.4f %-10.4f\n",
               stats[i].task_id, stats[i].tickets, stats[i].target_share,
               stats[i].mean_share, stats[i].std_dev, stats[i].abs_error);
        error_sum += stats[i].abs_error;
    }
    printf("  mean absolute error: %.4f", error_sum / tasks);
    if (limit >= 0.0) printf(" (limit %.2f)", limit);
    printf("\n\n");
    return error_sum / tasks;
}

#endif /* EXPERIMENT_STATS_H */
