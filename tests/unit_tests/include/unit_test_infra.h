/* ── Mini framework ─────────────────────────────────────────── */

#ifndef UNIT_TEST_INFRA_H
#define UNIT_TEST_INFRA_H

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static int name(void)

#define ASSERT(cond, msg)                                      \
    do {                                                       \
        if (!(cond)) {                                         \
            fprintf(stderr, "  ASSERT FAILED: %s\n", (msg));  \
            return 1;                                          \
        }                                                      \
    } while (0)

#define RUN(name)                                              \
    do {                                                       \
        tests_run++;                                           \
        printf("[ RUN  ] %s\n", #name);                       \
        if (name() == 0) {                                     \
            printf("[ OK   ] %s\n", #name);                   \
            tests_passed++;                                    \
        } else {                                               \
            printf("[ FAIL ] %s\n", #name);                   \
            tests_failed++;                                    \
        }                                                      \
    } while (0)

#endif /* UNIT_TEST_INFRA_H */