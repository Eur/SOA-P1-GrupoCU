#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "parser.h"
#include "double_linked_list.h"
#include "task.h"
#include "rng.h"
#include "scheduler.h"
#include "logger.h"

#define MAX_TASK_THREADS 25
static pthread_t threads_per_task[MAX_TASK_THREADS];

static uint64_t scheduler_calculate_cumulutive_tickets(struct node * head) {
    uint64_t cumulative_ticket_sum = 0;
    FOR_EACH_NODE(head, current_node) {

        /*
         * Only count the tickets of tasks that are in a state
         * equal to READY, because those are the only ones
         * ready to be scheduled.
         */
        if (task_is_eligible(current_node->data)) {
            cumulative_ticket_sum += current_node->tickets;
        }
    }

    return cumulative_ticket_sum;
}

static bool scheduler_all_tasks_finished(struct node * head) {
    FOR_EACH_NODE(head, current_node) {
        if (!task_is_finished(current_node->data)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Sorts the tasks based on their tickets and selects a winner.
 *
 * @details This function uses the XORShift 32 algorithm to generate a pseudorandom
 * number, which is then used to select a winner task based on the cumulative
 * tickets of the eligible tasks. The function iterates through the list of tasks,
 * summing their tickets until the cumulative sum exceeds the winner ticket number.
 */
static uint32_t scheduler_sort_winner_ticket(uint64_t max_ticket_range) {
    uint32_t winner_ticket = 0;

    if (rng_xorshift32_get(&winner_ticket) == false) {
        return 0;
    }

    winner_ticket = winner_ticket % max_ticket_range + 1;

    return winner_ticket;
}

/**
 * @brief This function will get the winning task based on the cumulative sum
 * of tickets
 *
 * @param head head of the list of tasks
 * @param winner_ticket the winner ticket from the sort
 * @return the node task holding the winner ticket
 */
static struct node * scheduler_winner_task(struct node * head, uint32_t winner_ticket) {
    uint64_t cumulative_ticket_sum = 0;
    FOR_EACH_NODE(head, current_node) {
        if (task_is_eligible(current_node->data)) {
            cumulative_ticket_sum += current_node->tickets;

            if (cumulative_ticket_sum >= winner_ticket) {
                return current_node;
            }
        }
    }

    /*
     * If the function reaches this point, no
     * task was elegible to be scheduled.
     */
    return NULL;
}

/**
 * @brief Wraps a function defined outside scheduler
 * just to preserver readiness while reading the code.
 *
 * @details This functions makes the main thread to wait until
 * there is no other worker thread running.
 *
 * @param task_list_head head of the list of tasks
 */
static void scheduler_main_thread_waits_until_no_running_workers(struct node *task_list_head){
    task_wait_until_no_running(task_list_head);
}

struct node* scheduler_init(const char * tasks_metadata_path, uint32_t rng_seed) {
    struct node *task_list_head = NULL;
    if (parser_load(tasks_metadata_path, &task_list_head) != 0) {
        fprintf(stderr, "Scheduler: Failed to load tasks from metadata file '%s'\n", tasks_metadata_path);
        return NULL;
    }

    rng_xorshift32_seed(rng_seed);

    int task_thread_creation_result = 0;
    // Initiating the threads per task:
    FOR_EACH_NODE(task_list_head, current_task_node) {
        task_thread_creation_result = pthread_create(&threads_per_task[current_task_node->id], NULL, task_do_work_unit, (void*)current_task_node);
        if (task_thread_creation_result != 0) {
            return NULL;
        }
    }
    return task_list_head;
}


void scheduler_main_loop(struct node *task_list_head) {
    int scheduler_sorts = 0;

    bool remaining_tasks = true;

    while (remaining_tasks) {
        /*
         * The first thing to do is to wait until the RUNNING
         * task has finished:
         *
         * Iteration 1: in the first iteration no RUNNING tasks
         * are executing because all tasks are initialized to
         * READY, so this is skiped.
         *
         * Iteration > 1: one task should be RUNNING, so the main
         * scheduler thread should wait until that task reports
         * the state to READY (because the quantum has finished,
         * or other policy has stopped the task but it has not
         * finished) or FINISHED.
         */
        scheduler_main_thread_waits_until_no_running_workers(task_list_head);

        /*
         * If there are no more tasks to address, then stops
         * this loop, so the scheduler goes thorugh the
         * deinit routine.
         */
        if (scheduler_all_tasks_finished(task_list_head)) {
            remaining_tasks = false;
            continue;
        }

        /*
         * Scheduler is ready to start the lottery the first time
         * or once again.
         * First, the scheduler needs to iterate over the tasks
         * that are in READY state and count the number of tickets
         * each one has.
         */
        uint64_t cumulative_ticket_sum =
            scheduler_calculate_cumulutive_tickets(task_list_head);

        /*
         * If the ticket sum is equal to 0, we cannot stop the
         * scheduler thread yet, we need to wait until the last
         * RUNNING task finishes, and depending on the mechanism
         * it could take more iterations of the scheduler. That is
         * why the scheduler needs to continue and start waiting at
         * the begining of this loop.
         */
        if (cumulative_ticket_sum == 0) {
            continue;
        }

        /*
         * The scheduler takes the cumulative ticket sum as
         * the max number to do the lottery:
         */
        uint32_t winner_ticket = scheduler_sort_winner_ticket(cumulative_ticket_sum);

        /*
         * Based on the winner ticket, the scheduler gets
         * the winner task, so we get a pointer, pointing
         * to that winner task:
         */
        struct node *winner_task = scheduler_winner_task(task_list_head, winner_ticket);

        if (winner_task == NULL) {
            fprintf(stdout, "Scheduler: No eligible tasks to schedule\n");
            break;
        }

        task_t * data_from_task = (task_t *)winner_task->data;

        /*
         * Log the sorted task, before sending it to RUNNING
         */
        LOG_EVENT(
            "[Lottery Sort] dispatch=%d, winner_id=%" PRIu32
            ", winning_ticket=%" PRIu32 ", active_tickets=%" PRIu32
            ", run_units=%d, completed_units=%" PRIu32 ", state=%s",
            data_from_task->dispatches,
            winner_task->id,
            winner_ticket,
            winner_task->tickets - data_from_task->work_units_done,
            0,
            data_from_task->work_units_done,
            task_state_enum_to_str(data_from_task->state));

        /*
         * Move the new winner task from READY to
         * RUNNING:
         * This is the moment when the scheduler finally
         * sends the sorted task from READY to RUNNING.
         * NOTE: internally the task handles when it
         * finished. At that point it automatically knows
         * if its state is READY or FINISHED, and the
         * scheduler thread is notified of this decision
         * at the begining of this loop where this
         * thread is set to wait.
         */
        task_transition_to_running(data_from_task);

        scheduler_sorts++;
    }
}

bool scheduler_deinit(struct node *task_list_head) {

    FOR_EACH_NODE(task_list_head, current_task_node) {
        pthread_join(threads_per_task[current_task_node->id], NULL);
    }

    if (dll_clean_list(&task_list_head) == false) {
        fprintf(stderr, "Scheduler: Failed to clean up task list\n");
        return false;
    }

    return true;
}

