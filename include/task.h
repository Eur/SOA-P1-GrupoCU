#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

/**
 * Lifecycle states of a scheduled task.
 * Valid transitions:
 * TASK_READY -> TASK_RUNNING (task_transition_to_running)
 * TASK_RUNNING -> TASK_READY (task_transition_to_ready)
 * TASK_RUNNING -> TASK_FINISHED (task_transition_to_finished)
 * TASK_FINISHED is terminal: no further transitions are allowed.
 */
typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_FINISHED
} task_state_t;

/**
 * Private computation state for the Leibniz π approximation.
 * Each task resumes from where it left off after a dispatch.
 *
 * sum: accumulated partial sum of the series.
 * term: value of the last term added.
 * j: next index to evaluate (starts at 0).
 */
typedef struct {
    double sum;
    double term;
    uint64_t j;
} pi_state_t;

/**
 * Represents a schedulable task in the lottery scheduler.
 *
 * work_units_done: number of work units already executed.
 * dispatches: number of lottery draws this task has won.
 * state: current lifecycle state (READY/RUNNING/FINISHED).
 * pi: private Leibniz series state resumed each dispatch.
 * mutex: protects all fields in this struct.
 */
typedef struct {
    uint32_t work_units_done;
    uint32_t dispatches;
    task_state_t state;
    pi_state_t pi;
    pthread_mutex_t mutex;
} task_t;

/**
 * @brief Allocates and initialises a new task.
 *
 * @details The task starts in TASK_READY state with all counters set to zero.
 *
 * @return Pointer to the new task, or NULL on allocation failure.
 */
task_t *task_create(void);

/**
 * @brief Destroys a task by destroying its mutex and freeing the memory.
 *
 * @param task: task to destroy. No-op if NULL.
 */
void task_destroy(task_t *task);

/**
 * @brief Transitions a task from TASK_READY to TASK_RUNNING.
 *
 * @details Increments the dispatch counter on success.
 *
 * @param task: task to transition.
 * @return true on success, false if the task is not in TASK_READY state.
 */
bool task_transition_to_running(task_t *task);

/**
 * @brief Transitions a task from TASK_RUNNING to TASK_READY.
 *
 * @details Called after a work unit completes but more work units remain.
 *
 * @param task: task to transition.
 * @return true on success, false if the task is not in TASK_RUNNING state.
 */
bool task_transition_to_ready(task_t *task);

/**
 * @brief Transitions a task from TASK_RUNNING to TASK_FINISHED.
 *
 * @details TASK_FINISHED is terminal: the task will never be scheduled again.
 *
 * @param task: task to transition.
 * @return true on success, false if the task is not in TASK_RUNNING state.
 */
bool task_transition_to_finished(task_t *task);

/**
 * @brief Returns whether a task is eligible to enter the lottery draw.
 *
 * @details A task is eligible if and only if its state is TASK_READY.
 *
 * @param task: task to check.
 * @return true if eligible, false otherwise.
 */
bool task_is_eligible(task_t *task);

#endif /* TASK_H */