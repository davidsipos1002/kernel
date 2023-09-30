#ifndef ALGORITHM_LINKED_LIST_H_INCL
#define ALGORITHM_LINKED_LIST_H_INCL

#include <stdint.h>

typedef struct 
{
    uint64_t key;
    uint64_t link;
} linked_list_node;

typedef struct
{
    uint64_t head;
    uint64_t last;
    uint64_t count;
} linked_list;

void linked_list_init(linked_list *list);
void linked_list_insert_front(linked_list *list, linked_list_node *node);
void linked_list_insert_rear(linked_list *list, linked_list_node *node);
linked_list_node* linked_list_find(linked_list *list, uint64_t key);
void linked_list_insert_after(linked_list *list, uint64_t after, linked_list_node *node);
void linked_list_delete_first(linked_list *list);
void linked_list_delete_last(linked_list *list);
uint8_t linked_list_delete(linked_list *list, uint64_t key);

#endif