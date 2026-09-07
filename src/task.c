#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"


static pthread_cond_t state_cond  = PTHREAD_COND_INITIALIZER;

task_t *task_create(void)
{
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (task == NULL) {
        fprintf(stderr, "Error allocating memory for task: %s\n", strerror(errno));
        return NULL;
    }
    task->work_units_done = 0;
    task->dispatches = 0;
    task->state = TASK_READY;
    task->pi.sum = 2.0;
    task->pi.term = 1.0;
    task->pi.j = 0;
    int err = pthread_mutex_init(&task->mutex, NULL);
    if (err != 0) {
        fprintf(stderr, "Error initializing mutex for task: %s\n", strerror(err));
        free(task);
        return NULL;
    }
    return task;
}

void task_destroy(task_t *task)
{
    if (task == NULL) {
        return;
    }
    pthread_mutex_destroy(&task->mutex);
    free(task);
}


bool task_transition_to_running(task_t *task)
{
    pthread_mutex_lock(&task->mutex);
    if (task->state != TASK_READY) {
        fprintf(stderr, "Error: task_transition_to_running called on a task that is not in TASK_READY state.\n");
        pthread_mutex_unlock(&task->mutex);
        return false;
    }
    task->state = TASK_RUNNING;
    task->dispatches++;
    pthread_mutex_unlock(&task->mutex);
    return true;    
}


bool task_transition_to_ready(task_t *task)
{
    pthread_mutex_lock(&task->mutex);
    if (task->state != TASK_RUNNING) {
        fprintf(stderr, "Error: task_transition_to_ready called on a task that is not in TASK_RUNNING state.\n");
        pthread_mutex_unlock(&task->mutex);
        return false;
    }
    task->state = TASK_READY;
    pthread_mutex_unlock(&task->mutex);
    return true;
}


bool task_transition_to_finished(task_t *task)
{

    pthread_mutex_lock(&task->mutex);
    if (task->state != TASK_RUNNING) {
        fprintf(stderr, "Error: task_transition_to_finished called on a task that is not in TASK_RUNNING state.\n");
        pthread_mutex_unlock(&task->mutex);
        return false;
    }
    task->state = TASK_FINISHED;
    pthread_mutex_unlock(&task->mutex);
    return true;
}


bool task_is_eligible(task_t *task)
{
    pthread_mutex_lock(&task->mutex);
    bool eligible = (task->state == TASK_READY);
    pthread_mutex_unlock(&task->mutex);
    return eligible;

}


void * task_do_work_unit(void * task){

    task_t * current_task = (task_t*)task;
    /*
     * Sleeps this thread if it is in READY state.
     * It uses a while, but this is not a busy wait
     * because on pthread_cond_wait call, the OS
     * sends this task to wait, and removes it from
     * CPU 
     */
    while (current_task->state == TASK_READY) {
        printf("Task is waiting\n");
        pthread_cond_wait(&state_cond, &current_task->mutex);
    }

    if (current_task->state == TASK_RUNNING) {
        printf("Task is running\n");
        pthread_mutex_unlock(&current_task->mutex);

        current_task->pi.j++; 
        current_task->pi.term *= ((2.0 * current_task->pi.j - 1.0) * (2.0 * current_task->pi.j - 1.0)) / ((2.0 * current_task->pi.j) * (2.0 * current_task->pi.j + 1.0));
        current_task->pi.sum += 2.0 * current_task->pi.term;
        current_task->work_units_done++;
    }

    return NULL;
}

char * task_state_enum_to_str(task_state_t task_state_enum) {
    switch(task_state_enum) {
        case TASK_READY:
            return "TASK_READY";
        case TASK_RUNNING:
            return "TASK_RUNNING";
        case TASK_FINISHED:
            return "TASK_FINISHED";
        default:
            return "TASK_INVALID_ST";
    }
}
