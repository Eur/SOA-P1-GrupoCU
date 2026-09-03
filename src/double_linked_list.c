// System includes
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Project includes
#include "double_linked_list.h"

void dll_init_list(struct node ** head) {
    *head = NULL;
}

bool dll_insert_node(struct node ** head,
                     void * data, uint32_t id,
                 uint32_t tickets,
                 uint32_t work_units) {

    struct node * new_node = (struct node *)malloc(sizeof(struct node));
    if (new_node == NULL) {
        return false;
    }

    new_node->data = data;
    new_node->id = id;
    new_node->tickets = tickets;
    new_node->work_units = work_units;

    /*
     * If the head is NULL, then this new element
     * will be the first element in the list, if
     * not then we need to append it at the end.
     */
    if (*head == NULL) {
        *head = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
    } else {
        struct node * current = *head;
        /*
         * Going through all the list to
         * insert the new element at the end.
         */
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
        new_node->next = NULL;
    }
    return true;
}


bool dll_remove_node(struct node ** head, uint32_t id) {
    struct node * current = *head;
    while (current != NULL) {
        if (current->id == id) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                *head = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            }
            free(current);
            return true;
        }
        current = current->next;
    }
    return false;
}

bool dll_clean_list(struct node ** head) {
    struct node * current = *head;
    while (current != NULL) {
        struct node * next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
    return true;
}

void dll_print_list(struct node * head) {
    printf("Current List:\n");
    if (head == NULL) {
        printf("The list is empty.\n");
        return;
    }
    struct node * current = head;
    while (current != NULL) {
        printf("Node ID: %u, Tickets: %u, Work Units: %u\n",
               current->id, current->tickets, current->work_units);
        current = current->next;
    }
}
