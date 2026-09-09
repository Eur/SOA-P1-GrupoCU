#ifndef PARSER_H
#define PARSER_H

#include "double_linked_list.h"


typedef enum {
    LOG_PATH = 0,

} PARSER_ENUM;

/**
 * Parses a CSV file with the following format:
 *
 *   task_id,tickets,work_units
 *   1,3,100
 *   2,5,200
 *   ...
 *
 * Validation rules enforced:
 *  - Number of tasks must be between 5 and 25 (inclusive).
 *  - task_id must be a positive integer (> 0) and unique within the file.
 *  - tickets must be a positive integer (> 0).
 *  - work_units must be a positive integer (> 0).
 *  - The sum of all tickets must not exceed UINT32_MAX (tracked in uint64_t).
 *
 * On success, inserts each task into the linked list pointed to by head
 * and returns 0.
 *
 * On any error, prints a descriptive message to stderr, frees any nodes
 * already inserted, and returns -1. No partial execution is started.
 *
 * @param filename  Path to the CSV input file.
 * @param head      Output: head of the parsed task list.
 * @return          0 on success, -1 on any error.
 */
int parser_load(const char *filename, struct node **head);

/**
 * @brief Iterate over the parameter list specified by the user
 * 
 * @details Also, this function validates if the arguments are valid
 * or not and behave accordingly to accept the command or show the help.
 *
 * @param argc parameter quantity
 * @param argv array with the parameters
 * @return 0 when successfully reads the parameters, non-zero otherwise.
 */
int parser_parameter_get(int argc, char *argv[]);

/**
 * @brief Prints the command-line usage and option descriptions.
 */
void parser_show_help_on_missing_param(char * missing_parameter);

/**
 * @brief Get the event logs path
 * @return a char with the path
 */
char * parser_log_path_get(void);


/**
 * @brief Get mode
 * @return a char with the mode
 */
char * parser_mode_get(void);

/**
 * @brief Get quantum
 * @return a uint32_t with the quantum
 */
uint32_t parser_quantum_get(void);

/**
 * @brief Get slice percentage
 * @return a float with the slice percentage
 */
float parser_slice_percentage_get(void);

/**
 * @brief Get seed
 * @return a uint32_t with the seed
 */
uint32_t parser_seed_get(void);

/**
 * @brief Get summary file path
 * @return a string with the summary file path
 */
char * parser_summary_path_get(void);

/**
 * @brief Get max_dispatches
 * @return a uint32_t with the max_dispatches
 */
uint32_t parser_max_dispatches_get(void);

/**
 * @brief Get the input file path
 * @return a char with the path
 */
char * parser_input_path_get(void);

#endif /* PARSER_H */