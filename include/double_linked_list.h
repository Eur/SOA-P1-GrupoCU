
#ifndef DOUBLE_LINKED_LIST_H
#define DOUBLE_LINKED_LIST_H


/**
 * This struct holds the data related to a task
 * to be scheduled. The node represents a part
 * of a double linked list of tasks.
 *
 * data: pointer to the task data
 * id: unique identifier for the task
 * tickets: number of tickets assigned to the task
 * work_units: number of work units the task has
 * next: pointer to the next node in the list
 * prev: pointer to the previous node in the list
 */
struct node {
    void * data;
    uint32_t id;
    uint32_t tickets;
    uint32_t work_units;
    struct node * next;
    struct node * prev;
};

/**
 * Initializes the double linked list by setting the head pointer to NULL.
 *
 * @param head: pointer to the head of the list
 */
void init_list(struct node ** head);

/**
 * Inserts a new node at the end of the double linked list.
 *
 * @param head: pointer to the head of the list
 * @param data: pointer to the data for the new node
 * @param id: unique identifier for the task
 * @param tickets: number of tickets assigned to the task
 * @param work_units: number of work units the task has
 * @return: true if the node was inserted successfully, false otherwise
 */
bool insert_node(struct node ** head, void * data, uint32_t id, uint32_t tickets, uint32_t work_units);

/**
 * Removes a node from the double linked list.
 *
 * @param head: pointer to the head of the list
 * @param id: unique identifier for the task
 * @return: true if the node was removed successfully, false otherwise
 */
bool remove_node(struct node ** head, uint32_t id);

/**
 * Cleans the double linked list by removing all nodes.
 *
 * @param head: pointer to the head of the list
 * @return: true if the list was cleaned successfully, false otherwise
 */
bool clean_list(struct node ** head);

/**
 * Prints the contents of the double linked list.
 *
 * @param head: pointer to the head of the list
 */
void print_list(struct node * head);

#endif // DOUBLE_LINKED_LIST_H