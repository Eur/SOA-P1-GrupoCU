#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "double_linked_list.h"

static pthread_cond_t state_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
    pthread_mutex_lock(&mutex);
    if (task->state != TASK_READY) {
        fprintf(stderr, "Error: task_transition_to_running called on a task that is not in TASK_READY state.\n");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    task->state = TASK_RUNNING;
    task->dispatches++;
    pthread_cond_broadcast(&state_cond);
    pthread_mutex_unlock(&mutex);
    return true;    
}


bool task_transition_to_ready(task_t *task)
{
    pthread_mutex_lock(&mutex);
    if (task->state != TASK_RUNNING) {
        fprintf(stderr, "Error: task_transition_to_ready called on a task that is not in TASK_RUNNING state.\n");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    task->state = TASK_READY;
    pthread_cond_broadcast(&state_cond);
    pthread_mutex_unlock(&mutex);
    return true;
}


bool task_transition_to_finished(task_t *task)
{

    pthread_mutex_lock(&mutex);
    if (task->state != TASK_RUNNING) {
        fprintf(stderr, "Error: task_transition_to_finished called on a task that is not in TASK_RUNNING state.\n");
        pthread_mutex_unlock(&task->mutex);
        return false;
    }
    task->state = TASK_FINISHED;
    pthread_cond_broadcast(&state_cond);
    pthread_mutex_unlock(&mutex);
    return true;
}


bool task_is_eligible(task_t *task)
{
    pthread_mutex_lock(&mutex);
    bool eligible = (task->state == TASK_READY);
    pthread_mutex_unlock(&mutex);
    return eligible;

}

bool task_is_finished(task_t *task)
{
    pthread_mutex_lock(&mutex);
    bool finished = (task->state == TASK_FINISHED);
    pthread_mutex_unlock(&mutex);
    return finished;
}

void task_wait_for_state_change(void)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&state_cond, &mutex);
    pthread_mutex_unlock(&mutex);
}


void * task_do_work_unit(void * task_node){
    struct node * current_task_node = (struct node *)task_node;
    task_t * current_task_data = (task_t*)current_task_node->data;
    for (;;) {
        pthread_mutex_lock(&mutex);
        while (current_task_data->state != TASK_RUNNING &&
               current_task_data->state != TASK_FINISHED) {
            // If the task is in READY:
            pthread_cond_wait(&state_cond, &mutex);
        }

        if (current_task_data->state == TASK_FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // If the task is in RUNNING:
        pthread_mutex_unlock(&mutex);

        /*
         * Uncomment the following line when work is
         * ready. This is evidence that the tasks are
         * being addressed, for debug purposes:
         */
        printf("Task %u is running\n", current_task_node->id);
        current_task_data->pi.j++;
        current_task_data->pi.term *=
            ((2.0 * current_task_data->pi.j - 1.0) *
             (2.0 * current_task_data->pi.j - 1.0)) /
            ((2.0 * current_task_data->pi.j) *
             (2.0 * current_task_data->pi.j + 1.0));
        current_task_data->pi.sum += 2.0 * current_task_data->pi.term;

        /*
         * Uncomment the following line, only when we are sure
         * how to increment the work units:
         */
        //current_task_data->work_units_done++;

        /*
         * Now that the worker is holding the mutex lets leverage on
         * this time window to change the state to the corresponding
         * one:
         */
        pthread_mutex_lock(&mutex);
        if (current_task_data->work_units_done >= current_task_node->work_units) {
            current_task_data->state = TASK_FINISHED;
        } else {
            current_task_data->state = TASK_READY;
        }
        pthread_cond_broadcast(&state_cond);
        pthread_mutex_unlock(&mutex);
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

void task_broadcast_signal_to_wake_threads(void) {
    pthread_cond_broadcast(&state_cond);
}
