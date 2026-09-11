
#ifndef LOGGER_H
#define LOGGER_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "task.h"

#define EVENTS_LOG_PATH_BUFF_SIZE 256
#define SUMMARY_LOG_PATH_BUFF_SIZE 256
#define TOD_BUFF_SIZE 80
#define LOG_EVENTS_DEFAULT_PATH "results/scheduler_events_%s.csv"
#define LOG_SUMMARY_DEFAULT_PATH "results/scheduler_summary_%s.csv"



/**
 * @brief This function creates the path tree for the event logs file
 * and the summary logs file
 *
 * @param custom_events_log_file_path char specifying the event logs path
 * @param custom_summary_log_file_path char specifying the summary logs path
 *
 * @return bool true if success on creating paths, false otherwise.
 */
bool logger_init_structure(const char *custom_events_log_file_path, const char * custom_summary_log_file_path);

/**
 * @brief This function get the ToD (time of the day)
 * and stamps it into the log file
 *
 * @details the caller is responible for closing the
 * file this function remains opened.
 *
 * @param eventlog boolean to determine if the stamp goes
 * into the eventlog or the summary log
 *
 * @return FILE pointer to the opened log file
 */
FILE * logger_log_tod(bool eventlog);

/**
 * @brief This function logs a formatted string into the
 * desired log file
 *
 * @param file pointer to the file to write to
 * @param format string with the format of the log
 * @param ... list of the arguments that fills the format
 * specified in the format string.
 */
void logger_log_msg(FILE * file, const char * format, ...);

/**
 * Public macro that is in charged of logging into the 
 * event log file with whatever the format is formed.
 */
#define LOG_EVENT(...) \
    do { \
        FILE * eventlog_file = logger_log_tod(true); \
        if (eventlog_file != NULL) { \
            logger_log_msg(eventlog_file, __VA_ARGS__); \
            fclose(eventlog_file); \
        } else { \
            fprintf(stderr, "Logger: Failed to open eventlog file': %s\n", strerror(errno)); \
        } \
    } while(0)




/**
 * @brief Writes the summary CSV with one row per task.
 * @details Called once after all tasks finish. Overwrites any previous file.
 * @param tasks  Array of task pointers.
 * @param count  Number of tasks.
 * @return true on success, false on I/O error.
 */
/**
 * Public macro to write the summary CSV.
 * Mirrors LOG_EVENT for API consistency.
 */
#define LOG_SUMMARY(tasks, count) logger_write_summary(tasks, count)

bool logger_write_summary(task_t **tasks, uint32_t count);
#endif /* LOGGER_H */