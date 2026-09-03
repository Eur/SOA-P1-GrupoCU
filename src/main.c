#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <stdio.h>   
#include "double_linked_list.h"
#include "parser.h"   


int main(void) {

    // TODO: delete this when the scheduler is implemented,
    // this is just a placeholder so all the team can test
    // that this is working in each dev environments.
    struct node *head;

/* Prueba 1: archivo válido */
    printf("=== base.csv ===\n");
    if (parser_load("tests/base.csv", &head) == 0) {
        print_list(head);
        clean_list(&head);
    }

    /* Prueba 2: archivo inválido */
    printf("\n=== validation_error.csv (debe fallar) ===\n");
    if (parser_load("tests/validation_error.csv", &head) != 0) {
        printf("Rechazado correctamente.\n");
    }


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
