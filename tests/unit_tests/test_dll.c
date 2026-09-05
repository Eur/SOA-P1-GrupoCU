
// System imports
#include <stdio.h>
#include <stdint.h>

// Local project imports
#include "double_linked_list.h"

// Unit test framework import
#include "unit_test_infra.h"


TEST(test_dll_insert_and_remove) {
    struct node *head;

    dll_init_list(&head);

    ASSERT(head == NULL, "head should be NULL after initialization");

    // Insert nodes
    ASSERT(dll_insert_node(&head, NULL, 1, 10, 100) == true, "insert node 1");
    ASSERT(dll_insert_node(&head, NULL, 2, 20, 200) == true, "insert node 2");
    ASSERT(dll_insert_node(&head, NULL, 3, 30, 300) == true, "insert node 3");

    // Check list integrity
    ASSERT(head != NULL && head->id == 1, "head should be node 1");
    ASSERT(head->next != NULL && head->next->id == 2, "second node should be node 2");
    ASSERT(head->next->next != NULL && head->next->next->id == 3, "third node should be node 3");
    ASSERT(head->next->next->next == NULL, "there should be no fourth node");

    struct node *found_node = NULL;
    ASSERT(dll_find_node(head, 2, &found_node) == true, "find node 2");
    ASSERT(found_node != NULL && found_node->id == 2, "found node should be node 2");
    ASSERT(dll_find_node(head, 4, &found_node) == false, "find non-existent node 4 should fail");
    ASSERT(found_node == NULL, "found node should be NULL for non-existent node");

    // Remove a node
    ASSERT(dll_remove_node(&head, 2) == true, "remove node 2");

    // Check list integrity after removal
    ASSERT(head != NULL && head->id == 1, "head should still be node 1");
    ASSERT(head->next != NULL && head->next->id == 3, "second node should now be node 3");
    ASSERT(head->next->next == NULL, "there should be no third node");

    ASSERT(dll_find_node(head, 2, &found_node) == false, "find removed node should fail");
    ASSERT(found_node == NULL, "found node should be NULL for removed node");

    // Clean up
    dll_clean_list(&head);
    return 0;
}

TEST(test_dll_remove_all_nodes) {
    struct node *head;

    dll_init_list(&head);

    // Insert nodes
    ASSERT(dll_insert_node(&head, NULL, 1, 10, 100) == true, "insert node 1");
    ASSERT(dll_insert_node(&head, NULL, 2, 20, 200) == true, "insert node 2");
    ASSERT(dll_insert_node(&head, NULL, 3, 30, 300) == true, "insert node 3");

    // Remove all nodes
    ASSERT(dll_clean_list(&head) == true, "clean list");

    // Check that the list is empty
    ASSERT(head == NULL, "head should be NULL after removing all nodes");
    ASSERT(dll_remove_node(&head, 1) == false, "remove node from empty list should fail");
    ASSERT(dll_remove_node(&head, 2) == false, "remove node from empty list should fail");
    ASSERT(dll_remove_node(&head, 3) == false, "remove node from empty list should fail");

    struct node *found_node = NULL;
    ASSERT(dll_find_node(head, 1, &found_node) == false, "find node in empty list should fail");
    ASSERT(dll_find_node(head, 2, &found_node) == false, "find node in empty list should fail");
    ASSERT(dll_find_node(head, 3, &found_node) == false, "find node in empty list should fail");

    return 0;
}


int main(void) {
    printf("=== Double Linked List — Unit tests ===\n\n");

    RUN(test_dll_insert_and_remove);
    RUN(test_dll_remove_all_nodes);
}
