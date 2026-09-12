#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>

#include "logger.h"

static char events_log_file_path[EVENTS_LOG_PATH_BUFF_SIZE];
static char summary_log_file_path[SUMMARY_LOG_PATH_BUFF_SIZE];

static bool logger_create_log_tree(const char * log_file_path) {
    if (log_file_path == NULL || log_file_path[0] == '\0') {
        return false;
    }

    size_t path_length = strlen(log_file_path);
    char *path = malloc(path_length + 1);
    if (path == NULL) {
        return false;
    }
    memcpy(path, log_file_path, path_length + 1);

    for (char *separator = path; *separator != '\0'; separator++) {
        if (*separator != '/') {
            continue;
        }

        *separator = '\0';
        if (path[0] != '\0' && strcmp(path, ".") != 0 &&
            mkdir(path, 0777) != 0 && errno != EEXIST) {
            free(path);
            return false;
        }
        *separator = '/';
    }

    FILE *log_file = fopen(path, "a");
    free(path);
    if (log_file == NULL) {
        return false;
    }

    fclose(log_file);
    return true;
}


static bool logger_tod_to_human_read_str(char * buffer_to_put_tod, bool file_path) {
    time_t todays_raw_time = time(NULL);
    struct tm *local_time = localtime(&todays_raw_time);

    int tod_str_result = -1;
    if (file_path == true) {
        tod_str_result = strftime(buffer_to_put_tod, TOD_BUFF_SIZE, "%Y-%m-%d", local_time);
    } else {
        tod_str_result = strftime(buffer_to_put_tod, TOD_BUFF_SIZE, "%Y-%m-%d-%H:%M:%S", local_time);
    }
     

    return tod_str_result != 0;
}

static bool logger_fill_events_log_file_path(const char * custom_events_log_file_path){

    int fill_events_log_file_result = -1;

    if (custom_events_log_file_path == NULL) {

        char time_of_day_buffer[TOD_BUFF_SIZE];

        if (logger_tod_to_human_read_str(time_of_day_buffer, true) == false) {
            return false;
        }

        fill_events_log_file_result = snprintf(events_log_file_path, sizeof(events_log_file_path), LOG_EVENTS_DEFAULT_PATH, time_of_day_buffer);
    } else {
        fill_events_log_file_result = snprintf(events_log_file_path, sizeof(events_log_file_path), "%s",custom_events_log_file_path);
    }

    return fill_events_log_file_result != 0;
}


static bool logger_fill_summary_log_file_path(const char *custom_summary_log_file_path) {
    int result = -1;
    if(custom_summary_log_file_path == NULL){
        char tod[TOD_BUFF_SIZE];
        if(!logger_tod_to_human_read_str(tod, true)){
            return false;
        }
        result = snprintf(summary_log_file_path, sizeof(summary_log_file_path),
                          LOG_SUMMARY_DEFAULT_PATH, tod);
    } else {
        result = snprintf(summary_log_file_path, sizeof(summary_log_file_path), "%s", custom_summary_log_file_path);
    }
    return result > 0;
}

bool logger_init_structure(const char *custom_events_log_file_path, const char * custom_summary_log_file_path) {

    if (logger_fill_events_log_file_path(custom_events_log_file_path) == false) {
        fprintf(stderr, "Logger: Failed filling event logs file buffer: '%d'\n", __LINE__);
        return false;
    }


    if (!logger_fill_summary_log_file_path(custom_summary_log_file_path)) {
        fprintf(stderr, "Logger: Failed filling summary log file buffer: '%d'\n", __LINE__);
        return false;
    }
    if (!logger_create_log_tree(summary_log_file_path)) {
        fprintf(stderr, "Logger: Failed creating summary log file path: '%d'\n", __LINE__);
        return false;
    }

    bool eventlog_path_tree_result = logger_create_log_tree(events_log_file_path);

    if (eventlog_path_tree_result == false) {
        fprintf(stderr, "Logger: Failed creating event logs file path: '%d'\n", __LINE__);
        return false;
    }
    return true;
}


FILE * logger_log_tod(bool eventlog) {
    char time_of_day_buffer[TOD_BUFF_SIZE];
    if (logger_tod_to_human_read_str(time_of_day_buffer, false) == false) {
        return NULL;
    }

    
    static bool events_initialized = false;
    const char *mode = (eventlog && !events_initialized) ? "w" : "a";
    if (eventlog) events_initialized = true;

    FILE *log_file = fopen(eventlog ? events_log_file_path : summary_log_file_path, mode);
    fprintf(log_file, "[%s]", time_of_day_buffer);
    fflush(log_file);

    /*
     * Not closing the file right now because we need it
     * opened in the next step, and the next step is
     * responsible for closing it.
     */
    return log_file;
}

void logger_log_msg(FILE * file, const char * format, ...) {
    va_list args;
    va_start(args, format);
    // vfprintf processes va_list arguments safely
    vfprintf(file, format, args);
    va_end(args);

    // Print a line jump always after the log
    fprintf(file, "\n");
    fflush(file);
}


bool logger_write_summary(struct node *list_head) {

    if (list_head == NULL) {
        return false;
    }

    uint64_t total_done = 0;
    FOR_EACH_NODE(list_head, n) {
        task_t *task = (task_t *)n->data;
        if (task == NULL) {
            continue;
        }
        total_done += task->work_units_done;
    }

    FILE *f = fopen(summary_log_file_path, "w");
    if (f == NULL) {
        fprintf(stderr, "Logger: Failed to open summary log file: %s\n", strerror(errno));
        return false;
    }

    fprintf(f, "task_id,tickets,work_units_assigned,work_units_completed,"
               "dispatches,first_dispatch,last_dispatch,pi_approx,observed_share\n");

    FOR_EACH_NODE(list_head, n) {
        task_t *task = (task_t *)n->data;
        if (task == NULL) {
            continue;
        }

        double pi_approx = task->pi.sum;
        double observed_share = (total_done > 0)
            ? (double)task->work_units_done / total_done
            : 0.0;

        fprintf(f, "%u,%u,%u,%u,%u,%u,%u,%.10f,%.6f\n",
            task->id,
            task->tickets,
            task->work_units,
            task->work_units_done,
            task->dispatches,
            task->first_dispatch,
            task->last_dispatch,
            pi_approx,
            observed_share);
    }

    fclose(f);
    return true;
}

