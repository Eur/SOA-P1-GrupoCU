#ifndef SCHEDULER_H
#define SCHEDULER_H

/**
 * @brief Initializes the scheduler.
 * 
 * @details This function will create and initialize
 * the necessary data structures for the scheduler
 * to function properly. The caller will be responsible
 * for freeing any resources allocated by this function.
 *
 * @param tasks_metadata_path The path to the tasks metadata file.
 * @return A pointer to the head of the task list if successful,
 * NULL otherwise.
 */
struct node* scheduler_init(const char * tasks_metadata_path, uint32_t rng_seed);
/**
 * @brief Configures cooperative mode (--slice-percent P).
 *
 * @details For each task computes slice = max(1, ceil(work_units * percent / 100))
 *          and calls task_set_slice. Must be called before scheduler_main_loop.
 *
 * @param task_list_head  List returned by scheduler_init.
 * @param percent         Value of P from --slice-percent.
 */
void scheduler_configure_cooperative(struct node *task_list_head, float percent);


/**
 * @brief Runs the scheduler.
 *
 * @details This function will run the scheduler and
 * execute any tasks that are ready to run. The caller
 * will be responsible for ensuring that this function
 * is called periodically to allow the scheduler to
 * execute tasks.
 */
void scheduler_run(void);

/**
 * @brief The main loop for the scheduler.
 *
 * @details This function will contain the main loop
 * for the scheduler, periodically calling the scheduler_run
 * function to execute tasks.
 */
void scheduler_main_loop(struct node *task_list_head);

/**
 * @brief Deinitializes the scheduler.
 *
 * @details This function will free any resources allocated
 * by the caller of the scheduler.
 *
 * @param task_list_head The head of the task list to be
 * deinitialized.
 * @return true if the scheduler was deinitialized successfully,
 * false otherwise.
 */
bool scheduler_deinit(struct node *task_list_head);

bool scheduler_has_running_task(struct node *task_list_head);

#endif // SCHEDULER_H
