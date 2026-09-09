
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

    char * input_file_path = parser_input_path_get();
    uint32_t seed = parser_seed_get();
    
    /*
     * Inject each of these parameters into scheduler_init
     * when required
     */

    /*char * mode = parser_mode_get();
    uint32_t quantum;
    float slice_percentage;
    if (strcmp(mode, "quantum") == 0) {
        quantum = parser_quantum_get();
    } else if (strcmp(mode, "cooperative") == 0) {
        slice_percentage = parser_slice_percentage_get();
    }
    char * summary_file_path = parser_summary_path_get();
    uint32_t max_dispatches __attribute__((unused)) = parser_max_dispatches_get();*/
    struct node *task_list_head = scheduler_init(input_file_path, seed);

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

// ./lottery_scheduler --input tests/base.csv --mode quantum --quantum 1 --seed 2026 --summary results/base_summary.csv --max-dispatches 10000
// ./lottery_scheduler --input tests/base.csv --mode cooperative --slice-percent 10 --seed 2026 --summary results/base_summary.csv
