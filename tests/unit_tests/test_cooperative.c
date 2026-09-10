// System imports
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Local project imports
#include "task.h"

// Unit test framework import
#include "unit_test_infra.h"


/* Helper that mirrors the integer ceiling used in scheduler_configure_cooperative */
static uint32_t slice_ceil(uint32_t work_units, uint32_t percent)
{
    uint64_t raw = ((uint64_t)work_units * percent + 99) / 100;
    return (raw >= 1) ? (uint32_t)raw : 1;
}


/* ── task_set_slice ─────────────────────────────────────────────────────── */

TEST(test_task_set_slice_normal)
{
    task_t task;
    memset(&task, 0, sizeof(task));

    task_set_slice(&task, 10);
    ASSERT(task.slice_size == 10, "slice_size should be 10");

    task_set_slice(&task, 1);
    ASSERT(task.slice_size == 1, "slice_size should be 1");

    task_set_slice(&task, 200);
    ASSERT(task.slice_size == 200, "slice_size should be 200");

    return 0;
}

TEST(test_task_set_slice_zero_clamps_to_one)
{
    task_t task;
    memset(&task, 0, sizeof(task));
    task.slice_size = 99;

    task_set_slice(&task, 0);
    ASSERT(task.slice_size == 1, "slice_size should be clamped to 1 when units == 0");

    return 0;
}


/* ── Ceiling arithmetic ─────────────────────────────────────────────────── */

TEST(test_slice_ceil_exact_division)
{
    /* Values from the default input file at 10% — all divide evenly */
    ASSERT(slice_ceil(100, 10) == 10, "100 wu x 10% = 10");
    ASSERT(slice_ceil(200, 10) == 20, "200 wu x 10% = 20");
    ASSERT(slice_ceil( 50, 10) ==  5,  "50 wu x 10% = 5");
    ASSERT(slice_ceil( 80, 10) ==  8,  "80 wu x 10% = 8");
    ASSERT(slice_ceil( 60, 10) ==  6,  "60 wu x 10% = 6");
    return 0;
}

TEST(test_slice_ceil_rounds_up)
{
    /* 101 x 10% = 10.1 -> 11 */
    ASSERT(slice_ceil(101, 10) == 11, "101 wu x 10% rounds up to 11");
    /* 11  x 10% = 1.1  -> 2  */
    ASSERT(slice_ceil( 11, 10) ==  2,  "11 wu x 10% rounds up to 2");
    /* 9   x 10% = 0.9  -> 1  */
    ASSERT(slice_ceil(  9, 10) ==  1,   "9 wu x 10% rounds up to 1 (minimum)");
    /* 1   x 10% = 0.1  -> 1  */
    ASSERT(slice_ceil(  1, 10) ==  1,   "1 wu x 10% rounds up to 1 (minimum)");
    return 0;
}

TEST(test_slice_ceil_full_percent)
{
    ASSERT(slice_ceil(100, 100) == 100, "100 wu x 100% = 100");
    ASSERT(slice_ceil( 77, 100) ==  77,  "77 wu x 100% = 77");
    ASSERT(slice_ceil(  1, 100) ==   1,   "1 wu x 100% = 1");
    return 0;
}

TEST(test_slice_ceil_minimum_enforced_at_zero_percent)
{
    ASSERT(slice_ceil(100, 0) == 1, "0% is clamped to minimum slice of 1");
    ASSERT(slice_ceil(  1, 0) == 1, "0% on 1 wu is still minimum 1");
    return 0;
}

TEST(test_slice_ceil_large_values)
{
    /* No overflow: 2^24 wu x 50% */
    uint32_t wu = 1u << 24; /* 16 777 216 */
    ASSERT(slice_ceil(wu, 50) == wu / 2, "large wu x 50% = wu/2");
    return 0;
}


int main(void)
{
    printf("=== Cooperative Mode — Unit tests ===\n\n");

    RUN(test_task_set_slice_normal);
    RUN(test_task_set_slice_zero_clamps_to_one);
    RUN(test_slice_ceil_exact_division);
    RUN(test_slice_ceil_rounds_up);
    RUN(test_slice_ceil_full_percent);
    RUN(test_slice_ceil_minimum_enforced_at_zero_percent);
    RUN(test_slice_ceil_large_values);
}