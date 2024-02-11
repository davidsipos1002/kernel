#ifndef ALGORITHM_XOR_LINKED_LIST_H_INCL
#define ALGORITHM_XOR_LINKED_LIST_H_INCL

#include <stdint.h>

typedef struct 
{
    uint64_t key;
    uint64_t link;
} xor_linked_list_node;

typedef struct
{
    uint64_t head;
    uint64_t last;
    uint64_t count;
} xor_linked_list;

void xor_linked_list_init(xor_linked_list *list);
void xor_linked_list_insert_front(xor_linked_list *list, xor_linked_list_node *node);
void xor_linked_list_insert_rear(xor_linked_list *list, xor_linked_list_node *node);
xor_linked_list_node* xor_linked_list_find(xor_linked_list *list, uint64_t key);
void xor_linked_list_insert_after(xor_linked_list *list, uint64_t after, xor_linked_list_node *node);
void xor_linked_list_delete_first(xor_linked_list *list);
void xor_linked_list_delete_last(xor_linked_list *list);
uint8_t xor_linked_list_delete(xor_linked_list *list, uint64_t key);

#endif