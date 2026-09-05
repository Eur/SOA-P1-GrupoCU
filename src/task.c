#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"


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


void task_do_work_unit(task_t *task){
    task->pi.j++; 
    task->pi.term *= ((2.0 * task->pi.j - 1.0) * (2.0 * task->pi.j - 1.0)) / ((2.0 * task->pi.j) * (2.0 * task->pi.j + 1.0));
    task->pi.sum += 2.0 * task->pi.term;
    task->work_units_done++;
}