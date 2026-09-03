// System includes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

// Project includes
#include "rng.h"
#include "double_linked_list.h"


int main(void) {

    /*
     * TODO: delete this when the scheduler is implemented,
     * this is just a placeholder so all the team can test
     * that this is working in each dev environments.
     */
    struct node *head;
    dll_init_list(&head);

    dll_insert_node(&head, NULL, 1, 10, 5);
    dll_insert_node(&head, NULL, 2, 20, 10);
    dll_print_list(head);
    dll_remove_node(&head, 1);
    dll_remove_node(&head, 2);
    dll_print_list(head);

    dll_insert_node(&head, NULL, 3, 30, 15);
    dll_insert_node(&head, NULL, 4, 40, 20);
    dll_print_list(head);
    dll_clean_list(&head);
    dll_print_list(head);

    /*
     * If this is compiled with "make asan" this will
     * trigger a report saying there is a memory leak,
     * because these two nodes are not freed.
     */
    dll_insert_node(&head, NULL, 5, 50, 25);
    dll_insert_node(&head, NULL, 6, 60, 30);

    //----------------------------
    // Testing random generator
    uint32_t seed = 12345;
    if (rng_xorshift32_seed(seed) == true) {
        uint32_t random_number;

        for (int i = 0; i < 2; i++) {
            if (rng_xorshift32_get(&random_number) == true) {
                printf("Try %i | Seed: %u | Random num: %u\n", i, seed, random_number);
                random_number = 0;
            }
        }

    }
    return 0;
}
