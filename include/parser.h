#ifndef PARSER_H
#define PARSER_H

#include "double_linked_list.h"

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

#endif /* PARSER_H */