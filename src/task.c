#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#include "task.h"
#include "double_linked_list.h"

static pthread_cond_t state_cond  = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

task_t *task_create(uint32_t id, uint32_t tickets, uint32_t work_units)
{
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (task == NULL) {
        fprintf(stderr, "Error allocating memory for task: %s\n", strerror(errno));
        return NULL;
    }
    task->id             = id;
    task->tickets        = tickets;
    task->work_units     = work_units;
    task->first_dispatch = 0;
    task->last_dispatch  = 0;
    task->work_units_done = 0;
    task->dispatches = 0;
    task->slice_size = 1; // Default slice size
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
    time_t now = time(NULL);
    if (task->dispatches == 1)
        task->first_dispatch = now;
    task->last_dispatch = now;
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
        pthread_mutex_unlock(&mutex);
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

void task_wait_until_no_running(struct node *task_list_head)
{
    /*
     * NOTE! This function is only executed by the scheduler thread.
     * This iterates over all tasks, when it finds a RUNNING task
     * the scheduler thread is sent to sleep and wait until that
     * task finishes.
     * When the RUNNING task finishes, the scheduler gets wake
     * and will iterate over all tasks again, finding that no
     * tasks are RUNNING, so the loop breaks and the
     * scheduler can continue its work.
     */
    pthread_mutex_lock(&mutex);
    /*
     * This is not a busy waiting as it is explained
     * further along this function:
     */
    for (;;) {
        bool running = false;
        FOR_EACH_NODE(task_list_head, current_task_node) {
            task_t *task_data = (task_t *)current_task_node->data;
            if (task_data->state == TASK_RUNNING) {
                /*
                 * Fine, scheduler found a RUNNING task,
                 * lets send it to sleep and wait
                 */
                running = true;
                break;
            }
        }

        /*
         * If the scheduler did not find any RUNNING task
         * then we can break this main loop, so the scheduler
         * can resume its main routin, and perform a new
         * lottery:
         */
        if (!running) {
            break;
        }
        /*
         * The scheduler is sleep and waiting,
         * also right now the scheduler has
         * released the lock.
         */
        pthread_cond_wait(&state_cond, &mutex);

        /*
         * At this point the thread gets wake, so it
         * will continue the main loop:
         */
    }

    /*
     * If the scheduler has just breake the loop
     * then we need to unlock the mutex:
     */
    pthread_mutex_unlock(&mutex);
}

void task_set_slice(task_t *task, uint32_t slice_size) {
    task -> slice_size = (slice_size >=1) ? slice_size : 1;
}

void * task_do_work_unit(void * task_node) {
    /*
     * NOTE: This is the function that all worker threads
     * execute.
     */
    struct node * current_task_node = (struct node *)task_node;
    task_t * current_task_data = (task_t*)current_task_node->data;
    /*
     * We need to mantain the thread alive in a loop,
     * but this is not busy-waiting as it is explained
     * further in this function: 
     */
    for (;;) {
        /*
         * Each worker will try to get the mutex, so
         * the first one able to lock the mutex enters
         * the critical region, the other workers
         * wait:
         */
        pthread_mutex_lock(&mutex);

        while (current_task_data->state != TASK_RUNNING &&
               current_task_data->state != TASK_FINISHED) {
            /*
             * If the task is in READY, this worker is sent
             * to wait and sleep until this task wins the
             * lottery:
             */ 
            pthread_cond_wait(&state_cond, &mutex);
        }

        if (current_task_data->state == TASK_FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        

        /*
         * Unlocking the mutex here should be super safe
         * because no other task is in RUNNING, and
         * it is needed because the scheduler needs
         * to hold the mutex to do operations such
         * as iterate over the list of tasks and see
         * if they are RUNNING or not:
         */
        pthread_mutex_unlock(&mutex);

        uint32_t units_run = 0;
        while (units_run < current_task_data->slice_size &&
               current_task_data->work_units_done < current_task_node->work_units) {
            current_task_data->pi.j++;
            current_task_data->pi.term *=
                ((2.0 * (double)current_task_data->pi.j - 1.0) *
                 (2.0 * (double)current_task_data->pi.j - 1.0)) /
                ((2.0 * (double)current_task_data->pi.j) *
                 (2.0 * (double)current_task_data->pi.j + 1.0));
            if (!isfinite(current_task_data->pi.term) ||
                !isfinite(current_task_data->pi.sum)) {
                fprintf(stderr, "Task %u: floating point overflow in pi computation at j=%" PRIu64 "\n",
                        current_task_data->id, current_task_data->pi.j);
                break;
            }
            current_task_data->pi.sum += 2.0 * current_task_data->pi.term;
            current_task_data->work_units_done++;
            units_run++;
        }

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

bool task_is_there_any_running_task(struct node *task_list_head) {
    pthread_mutex_lock(&mutex);
    FOR_EACH_NODE(task_list_head, current_task_node) {
        task_t * task_data = (task_t *)current_task_node->data;
        if(task_data->state == TASK_RUNNING) {
            pthread_mutex_unlock(&mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&mutex);
    return false;
}
