
#ifndef LOGGER_H
#define LOGGER_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define EVENTS_LOG_PATH_BUFF_SIZE 256
#define SUMMARY_LOG_PATH_BUFF_SIZE 256
#define TOD_BUFF_SIZE 80
#define LOG_EVENTS_DEFAULT_PATH "results/scheduler_events_%s.csv"
#define LOG_SUMMARY_DEFAULT_PATH "results/scheduler_summary_%s.csv"




bool logger_init_structure(const char *custom_events_log_file_path, const char * custom_summary_log_file_path);

FILE * logger_log_tod(bool eventlog);

void logger_log_msg(FILE * file, const char * format, ...);

#define LOG_EVENT(format, ...) \
    do { \
        FILE * eventlog_file = logger_log_tod(true); \
        if (eventlog_file != NULL) { \
            logger_log_msg(eventlog_file, format, ##__VA_ARGS__); \
            fclose(eventlog_file); \
        } else { \
            fprintf(stderr, "Logger: Failed to open eventlog file': %s\n", strerror(errno)); \
        } \
    } while(0)

#endif /* LOGGER_H */