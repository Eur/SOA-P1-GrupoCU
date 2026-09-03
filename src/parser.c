#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define MIN_TASKS  5
#define MAX_TASKS  25
#define LINE_BUF   512

static int id_exists(struct node *head, uint32_t id)
{
    for (struct node *cur = head; cur != NULL; cur = cur->next) {
        if (cur->id == id) return 1;
    }
    return 0;
}

int parser_load(const char *filename, struct node **head)
{
    init_list(head);
    FILE *file = fopen(filename, "r");
    
    if (!file) 
    {
        fprintf(stderr, "Parser opening file '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    char line[LINE_BUF];

    // Read the header line
    if (!fgets(line, sizeof(line), file)) {
        fprintf(stderr, "Parser reading header from file '%s'\n", filename);
        fclose(file);
        return -1;
    }

    int n = 0;
    int lineno = 1;
    uint64_t total_tickets = 0;

    while(fgets(line, sizeof(line), file)) {

        lineno++;

        line[strcspn(line, "\r\n")] = '\0'; // Remove newline characters

        if (strlen(line) == 0) {
            continue; // Skip empty lines
        }

        if(n >= MAX_TASKS) {
            fprintf(stderr, "Parser: Too many tasks in file '%s' (line %d) maximum allowed is %d\n", filename, lineno, MAX_TASKS );
            goto err;
        }

        long id_line, tickets_line, work_units_line;
        if(sscanf(line, "%ld,%ld,%ld", &id_line, &tickets_line, &work_units_line) != 3) {
            fprintf(stderr,
                    "Parser: invalid format at line %d: '%s'\n"
                    "        expected: task_id,tickets,work_units\n",
                    lineno, line);
            goto err;
        }

        if(id_line <=0) {
            fprintf(stderr, "Parser: invalid task_id at line %d: '%s'\n"
                            "        task_id must be a positive integer\n",
                    lineno, line);
            goto err;
        }
        if(tickets_line <=0) {
            fprintf(stderr, "Parser: invalid tickets at line %d: '%s'\n"
                            "        tickets must be a positive integer\n",
                    lineno, line);
            goto err;
        }
        if(work_units_line <=0) {
            fprintf(stderr, "Parser: invalid work_units at line %d: '%s'\n"
                            "        work_units must be a positive integer\n",
                    lineno, line);
            goto err;
        }

        if(id_line > (long)UINT32_MAX || tickets_line > (long)UINT32_MAX || work_units_line > (long)UINT32_MAX) {
            fprintf(stderr, "Parser: value out of range at line %d: '%s'\n"
                            "        values must be <= %u\n",
                    lineno, line, UINT32_MAX);
            goto err;
        }

        uint32_t id = (uint32_t)id_line;
        uint32_t tickets = (uint32_t)tickets_line;
        uint32_t work_units = (uint32_t)work_units_line;    

        if(id_exists(*head, id)) {
            fprintf(stderr, "Parser: duplicate task_id at line %d: '%s'\n"
                            "        task_id must be unique\n",
                    lineno, line);
            goto err;
        }
        total_tickets += tickets;
        if(total_tickets > UINT32_MAX) {
            fprintf(stderr, "Parser: total tickets exceed UINT32_MAX at line %d: '%s'\n", lineno, line);
            goto err;
        }
        if(!insert_node(head, NULL, id, tickets, work_units)) {
            fprintf(stderr, "Parser: failed to insert node at line %d: '%s'\n", lineno, line);
            goto err;
        }
        n++;
    }

    fclose(file);

    if(n < MIN_TASKS) {
        fprintf(stderr, "Parser: Too few tasks in file '%s' (found %d, minimum required is %d)\n", filename, n, MIN_TASKS);
        clean_list(head);
        return -1;
    }
    return 0;

    err:
    
        fclose(file);
        clean_list(head);
        return -1;
}