// System includes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Project includes
#include "double_linked_list.h"
#include "scheduler.h"
#include "logger.h"
#include "parser.h"


int main(int argc, char *argv[]) {

    int parser_result = parser_parameter_get(argc, argv);
    if (parser_result != 0){
        return 1;
    }

    char *input_file_path   = parser_input_path_get();
    uint32_t seed           = parser_seed_get();
    char *mode              = parser_mode_get();
    char *log_event_path    = parser_log_path_get();
    char *summary_file_path = parser_summary_path_get();

    struct node *task_list_head = scheduler_init(input_file_path, seed);
    if (task_list_head == NULL) {
        return 1;
    }

    if (strcmp(mode, "cooperative") == 0) {
        uint32_t percent = (uint32_t)parser_slice_percentage_get();
        scheduler_configure_cooperative(task_list_head, percent);
    }
    /* quantum: Issue #8 */

    logger_init_structure(log_event_path, summary_file_path);

    scheduler_main_loop(task_list_head);

    task_t *tasks[32];
    uint32_t task_count = 0;
    FOR_EACH_NODE(task_list_head, n) {
        if (task_count < 32)
            tasks[task_count++] = (task_t *)n->data;
    }
    logger_write_summary(tasks, task_count);

    if (scheduler_deinit(task_list_head) == false) {
        return 1;
    }
    return 0;
}