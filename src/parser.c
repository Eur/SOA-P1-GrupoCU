#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include <sys/stat.h>

#include "task.h"
#include "parser.h"

#define MIN_TASKS  5
#define MAX_TASKS  25
#define LINE_BUF   512

static char * input_file_path;
static char * mode;
static uint32_t quantum = 0;
static float slice_percent = 0.0f;
static uint32_t seed = 0;
static char * log_events_path;
static char * summary_file_path;
static uint32_t max_dispatches = 0;

static int parser_parse_uint32(const char *value, uint32_t *result)
{
    char *endptr;
    unsigned long parsed_value;

    errno = 0;
    parsed_value = strtoul(value, &endptr, 10);
    if (errno != 0 || *value == '\0' || *endptr != '\0' ||
        parsed_value == 0 || parsed_value > UINT32_MAX) {
        return 1;
    }

    *result = (uint32_t)parsed_value;
    return 0;
}

static int parser_parse_percentage(const char *value, float *result)
{
    char *endptr;
    float parsed_value;

    errno = 0;
    parsed_value = strtof(value, &endptr);
    if (errno != 0 || *value == '\0' || *endptr != '\0' ||
        !isfinite(parsed_value) || parsed_value <= 0.0f ||
        parsed_value > 100.0f) {
        return 1;
    }

    *result = parsed_value;
    return 0;
}

static int parser_is_valid_mode(const char *value)
{
    return strcmp(value, "quantum") == 0 || strcmp(value, "cooperative") == 0;
}

static int parser_is_valid_path_string(const char *path)
{
    if (path == NULL || *path == '\0') {
        return 0;
    }

    for (const unsigned char *character = (const unsigned char *)path;
         *character != '\0'; character++) {
        if (*character < 32) {
            return 0;
        }
    }

    return 1;
}

static int parser_is_existing_input_file(const char *path)
{
    struct stat path_info;

    return parser_is_valid_path_string(path) &&
           stat(path, &path_info) == 0 && S_ISREG(path_info.st_mode);
}

static int parser_has_existing_parent_directory(const char *path)
{
    char *last_separator;
    char parent_path[LINE_BUF];
    struct stat path_info;

    if (!parser_is_valid_path_string(path) || strlen(path) >= sizeof(parent_path)) {
        return 0;
    }

    strcpy(parent_path, path);
    last_separator = strrchr(parent_path, '/');
    if (last_separator == NULL) {
        strcpy(parent_path, ".");
    } else if (last_separator == parent_path) {
        last_separator[1] = '\0';
    } else {
        *last_separator = '\0';
    }

    return stat(parent_path, &path_info) == 0 && S_ISDIR(path_info.st_mode);
}

int parser_load(const char *filename, struct node **head)
{
    dll_init_list(head);
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

        if (dll_find_node(*head, id, NULL)) {
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
        
        /*
         * Creating a task for each node.
         * NOTE: This should be freed inside the
         * double linked list infraestructure.
         */
        task_t *task = task_create(id, tickets, work_units);
        if(!dll_insert_node(head, task, id, tickets, work_units)) {
            fprintf(stderr, "Parser: failed to insert node at line %d: '%s'\n", lineno, line);
            goto err;
        }
        n++;
    }

    fclose(file);

    if(n < MIN_TASKS) {
        fprintf(stderr, "Parser: Too few tasks in file '%s' (found %d, minimum required is %d)\n", filename, n, MIN_TASKS);
        dll_clean_list(head);
        return -1;
    }
    return 0;

    err:
    
        fclose(file);
        dll_clean_list(head);
        return -1;
}

static int parser_parameter_validation(void) {
    if (!parser_is_existing_input_file(input_file_path)) {
        parser_show_help_on_missing_param("--input");
        return 1;
    }

    if (mode == NULL || !parser_is_valid_mode(mode)) {
        parser_show_help_on_missing_param("--mode");
        return 1;
    }

    if (strcmp(mode, "quantum") == 0) {
        if (quantum == 0) {
            parser_show_help_on_missing_param("--quantum");
            return 1;
        }
    } else if (strcmp(mode, "cooperative") == 0) {
        if (!(slice_percent > 0.0f && slice_percent <= 100.0f)) {
            parser_show_help_on_missing_param("--slice-percent");
            return 1;
        }
    }

    if (seed == 0) {
        parser_show_help_on_missing_param("--seed");
        return 1;
    }

    if (!parser_has_existing_parent_directory(summary_file_path)) {
        parser_show_help_on_missing_param("--summary");
        return 1;
    }

    if (log_events_path != NULL &&
        !parser_has_existing_parent_directory(log_events_path)) {
        parser_show_help_on_missing_param("--log");
        return 1;
    }

    return 0;
}


int parser_parameter_get(int argc, char *argv[]) {
    struct option long_options[] = {
        {"input", required_argument, NULL, 'i'},
        {"mode", required_argument, NULL, 'm'},
        {"quantum", required_argument, NULL, 'q'},
        {"slice-percent", required_argument, NULL, 'p'},
        {"seed", required_argument, NULL, 's'},
        {"log", required_argument, NULL, 'l'},
        {"summary", required_argument, NULL, 'z'},
        {"max-dispatches", required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0},
    };


    
    int option;
    while ((option = getopt_long(argc, argv, "imqpslzd:h", long_options, NULL)) != -1){
        switch (option) {
            case 'i':
                input_file_path = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            case 'q':
                if (parser_parse_uint32(optarg, &quantum) != 0) {
                        fprintf(stderr, "Invalid quantum: '%s'\n", optarg);
                        parser_show_help_on_missing_param("--quantum");
                        return 1;
                }
                break;
            case 'p':
                if (parser_parse_percentage(optarg, &slice_percent) != 0) {
                        fprintf(stderr, "Invalid slice percentage: '%s'\n", optarg);
                        parser_show_help_on_missing_param("--slice-percent");
                        return 1;
                }
                break;
            case 's':
                if (parser_parse_uint32(optarg, &seed) != 0) {
                        fprintf(stderr, "Invalid seed: '%s'\n", optarg);
                        parser_show_help_on_missing_param("--seed");
                        return 1;
                }
                break;
            case 'l':
                /*
                 * This won't crash and it is valid because optarg
                 * is a pointer already owned by the program, and
                 * will remain valid all the lifecycle of the execution.
                 */
                log_events_path = optarg;
                break;

            case 'z':
                summary_file_path = optarg;
                break;
            case 'd':
                if (parser_parse_uint32(optarg, &max_dispatches) != 0) {
                        fprintf(stderr, "Invalid max dispatches: '%s'\n", optarg);
                        parser_show_help_on_missing_param("--max-dispatches");
                        return 1;
                }
                break;
            case 'h':
                parser_show_help_on_missing_param(NULL);
                return 0;
            case '?':
                return 1;
            default:
                break;
        }
        
    }
    return parser_parameter_validation();
}



char * parser_input_path_get(void) {
    return input_file_path;
}

char * parser_mode_get(void) {
    return mode;
}

uint32_t parser_quantum_get(void) {
    return quantum;
}

float parser_slice_percentage_get(void) {
    return slice_percent;
}

uint32_t parser_seed_get(void) {
    return seed;
}

char * parser_log_path_get(void) {
    return log_events_path;
}

char * parser_summary_path_get(void) {
    return summary_file_path;
}

uint32_t parser_max_dispatches_get(void) {
    return max_dispatches;
}

void parser_show_help_on_missing_param(char * missing_parameter) {

    if (missing_parameter != NULL) {
        printf("\033[31mMissing parameter: %s\033[0m\n\n", missing_parameter);
    }

    static const char help_buffer[] =
        "Usage: ./lottery_scheduler [OPTIONS]\n"
        "\n"
        "Run the lottery scheduler with tasks loaded from a CSV file.\n"
        "\n"
        "Options:\n"
        "  -i, --input FILE          CSV file containing the tasks to schedule.\n"
        "  -m, --mode MODE           Scheduling mode (for example: quantum).\n"
        "  -q, --quantum UNITS       Number of work units in each quantum.\n"
        "  -p, --slice-percent PCT   Percentage of work used for each time slice.\n"
        "  -s, --seed VALUE          Seed used by the pseudo-random lottery.\n"
        "  -l, --log FILE            File where scheduler events are written.\n"
        "  -z, --summary FILE        File where the execution summary is written.\n"
        "  -d, --max-dispatches N    Maximum number of scheduler dispatches.\n"
        "  -h, --help                Show this help message.\n"
        "\n"
        "The input CSV must contain task_id, tickets, and work_units columns.\n"
        "\n"
        "Example:\n"
        "  ./lottery_scheduler --input tests/base.csv --mode quantum --quantum 1000 --seed 2026 --log results/base_events.csv --summary results/base_summary.csv\n";

    fputs(help_buffer, stdout);
}
