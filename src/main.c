#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "double_linked_list.h"


int main(void) {

    // TODO: delete this when the scheduler is implemented,
    // this is just a placeholder so all the team can test
    // that this is working in each dev environments.
    struct node *head;
    init_list(&head);

    insert_node(&head, NULL, 1, 10, 5);
    insert_node(&head, NULL, 2, 20, 10);
    print_list(head);
    remove_node(&head, 1);
    remove_node(&head, 2);
    print_list(head);

    insert_node(&head, NULL, 3, 30, 15);
    insert_node(&head, NULL, 4, 40, 20);
    print_list(head);
    clean_list(&head);
    print_list(head);

    // If this is compiled with "make asan" this will
    // trigger a report saying there is a memory leak,
    // beacuse these two nodes are not freed.
    insert_node(&head, NULL, 5, 50, 25);
    insert_node(&head, NULL, 6, 60, 30);
}
