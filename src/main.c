
// System includes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Project includes
#include "scheduler.h"
#include "logger.h"
#include "parser.h"


int main(int argc, char *argv[]) {

    int parser_result = parser_parameter_get(argc, argv);
    if (parser_result != 0){
        return 1;
    }

    uint32_t seed = 2026;
    struct node *task_list_head = scheduler_init("tests/base.csv", seed);

    if (task_list_head == NULL) {
        return 1;
    }

    char * log_event_path = parser_log_path_get();
    logger_init_structure(log_event_path, NULL);

    scheduler_main_loop(task_list_head);

    if (scheduler_deinit(task_list_head) == false) {
        return 1;
    }
    return 0;
}
