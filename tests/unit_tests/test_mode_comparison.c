// System imports
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Unit test framework import
#include "unit_test_infra.h"

#define SCHEDULER_BINARY "../../lottery_scheduler"
#define INPUT_FILE       "../mode_comparison_input_files/mode_comparison_input_file.csv"
#define OUTPUT_DIR       "/tmp/test_mode_comparison"
#define SEED             2026
#define SLICE_PERCENT    10
#define QUANTUM          1000
#define MAX_TASKS        25

typedef struct {
    uint32_t task_id;
    uint32_t tickets;
    uint64_t assigned;
    uint64_t completed;
    uint32_t dispatches;
    double   pi;
} row_t;

/* Reads a summary CSV into rows. Returns the row count, or -1 on failure. */
static int read_rows(const char *path, row_t *rows)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) return -1;

    char line[512];
    if (fgets(line, sizeof(line), f) == NULL) { fclose(f); return -1; } /* header */

    int n = 0;
    while (n < MAX_TASKS && fgets(line, sizeof(line), f) != NULL) {
        char *field = strtok(line, ",");
        for (int col = 0; field != NULL; col++, field = strtok(NULL, ",")) {
            if (col == 0) rows[n].task_id    = (uint32_t)strtoul(field, NULL, 10);
            if (col == 1) rows[n].tickets    = (uint32_t)strtoul(field, NULL, 10);
            if (col == 2) rows[n].assigned   = strtoull(field, NULL, 10);
            if (col == 3) rows[n].completed  = strtoull(field, NULL, 10);
            if (col == 4) rows[n].dispatches = (uint32_t)strtoul(field, NULL, 10);
            if (col == 7) rows[n].pi         = atof(field);
        }
        n++;
    }
    fclose(f);
    return n;
}

/* Runs the scheduler to completion (no --max-dispatches) in the given mode. */
static int run_mode(const char *name, const char *mode_args, row_t *rows)
{
    char cmd[1024], summary[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", OUTPUT_DIR);
    if (system(cmd) != 0) return -1;

    snprintf(summary, sizeof(summary), "%s/%s.csv", OUTPUT_DIR, name);
    snprintf(cmd, sizeof(cmd), "%s --input %s %s --seed %d --summary %s > /dev/null",
             SCHEDULER_BINARY, INPUT_FILE, mode_args, SEED, summary);
    if (system(cmd) != 0) return -1;

    return read_rows(summary, rows);
}

static uint64_t total_completed(const row_t *rows, int n)
{
    uint64_t sum = 0;
    for (int i = 0; i < n; i++) sum += rows[i].completed;
    return sum;
}

static uint32_t total_dispatches(const row_t *rows, int n)
{
    uint32_t sum = 0;
    for (int i = 0; i < n; i++) sum += rows[i].dispatches;
    return sum;
}

TEST(test_cooperative_vs_quantum_same_seed)
{
    row_t coop[MAX_TASKS], quant[MAX_TASKS];
    int nc = run_mode("cooperative", "--mode cooperative --slice-percent " "10", coop);
    int nq = run_mode("quantum", "--mode quantum --quantum " "1000", quant);
    ASSERT(nc == 5 && nq == 5, "both modes should report 5 tasks");

    printf("\n  Mode comparison (seed %d, cooperative slice %d%%, quantum %d)\n",
           SEED, SLICE_PERCENT, QUANTUM);
    printf("  %-6s %-10s %-12s %-12s %-14s\n",
           "task", "work", "disp_coop", "disp_quantum", "pi");

    for (int i = 0; i < nc; i++) {
        printf("  %-6u %-10llu %-12u %-12u %-14.10f\n", coop[i].task_id,
               (unsigned long long)coop[i].assigned, coop[i].dispatches,
               quant[i].dispatches, coop[i].pi);

        ASSERT(coop[i].task_id == quant[i].task_id, "task order should match");
        ASSERT(coop[i].completed == coop[i].assigned,
               "cooperative: task should complete all its work");
        ASSERT(quant[i].completed == quant[i].assigned,
               "quantum: task should complete all its work");
        ASSERT(fabs(coop[i].pi - quant[i].pi) < 1e-12,
               "pi should be the same in both modes");
        ASSERT(coop[i].dispatches == 100 / SLICE_PERCENT,
               "cooperative: 10% slices need exactly 10 dispatches per task");
        ASSERT(quant[i].dispatches == (coop[i].assigned + QUANTUM - 1) / QUANTUM,
               "quantum: dispatches should be ceil(work / quantum)");
    }

    uint64_t work_c = total_completed(coop, nc), work_q = total_completed(quant, nc);
    uint32_t disp_c = total_dispatches(coop, nc), disp_q = total_dispatches(quant, nc);

    printf("\n  %-26s %-12s %-12s\n", "metric", "cooperative", "quantum");
    printf("  %-26s %-12llu %-12llu\n", "total work completed",
           (unsigned long long)work_c, (unsigned long long)work_q);
    printf("  %-26s %-12u %-12u\n", "total dispatches", disp_c, disp_q);
    printf("  %-26s %-12.1f %-12.1f\n\n", "work units per dispatch",
           (double)work_c / disp_c, (double)work_q / disp_q);

    ASSERT(work_c == work_q, "total completed work should be equal in both modes");
    ASSERT(disp_c < disp_q, "quantum should need more dispatches than cooperative");
    return 0;
}

int main(void)
{
    printf("=== Mode comparison (case 6) — Unit tests ===\n");

    RUN(test_cooperative_vs_quantum_same_seed);

    printf("\nSummary: %d run, %d passed, %d failed\n",
           tests_run, tests_passed, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
