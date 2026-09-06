
// System includes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Project includes
#include "scheduler.h"


int main(void) {

    uint32_t seed = 2026;
    struct node *task_list_head = scheduler_init("tests/base.csv", seed);

    if (task_list_head == NULL) {
        return 1;
    }

    scheduler_main_loop(task_list_head);

    if (scheduler_deinit(task_list_head) == false) {
        return 1;
    }
    return 0;
}