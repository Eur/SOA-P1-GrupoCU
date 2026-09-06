
#ifndef LOGGER_H
#define LOGGER_H

#define LOG_RESULTS_DEFAULT_PATH "results/timeline.csv"

static char * log_results_dir = NULL;
static char * log_results_file = NULL;

#define LOG_DECISION(file, winner_id, winner_tickets, total_tickets) \
    do { \
        FILE *log_file = fopen(file, "a"); \
        if (log_file) { \
            fprintf(log_file, "Winner Task ID: %u, Winner Tickets: %u, Total Tickets: %llu\n", \
                    winner_id, winner_tickets, total_tickets); \
            fclose(log_file); \
        } else { \
            fprintf(stderr, "Logger: Failed to open log file '%s': %s\n", file, strerror(errno)); \
        } \
    } while (0)



bool logger_init_structure(const char *log_file_path);

#endif /* LOGGER_H */