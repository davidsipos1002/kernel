#ifndef ALGORITHM_DOUBLE_LINKED_LIST_H_INCL
#define ALGORITHM_DOUBLE_LINKED_LIST_H_INCL

#include <stdint.h>

typedef struct double_linked_list_node
{
    struct double_linked_list_node *prev;
    struct double_linked_list_node *next;
} double_linked_list_node;

typedef struct
{
    double_linked_list_node *head;
    uint64_t length;
} double_linked_list;

void double_linked_list_init(double_linked_list *list);
void double_linked_list_insert_front(double_linked_list *list, double_linked_list_node *node);
double_linked_list_node *double_linked_list_remove_front(double_linked_list *list);
void double_linked_list_insert_back(double_linked_list *list, double_linked_list_node *node);
double_linked_list_node *double_linked_list_remove_back(double_linked_list *list);
void double_linked_list_insert_after(double_linked_list *list, double_linked_list_node *after, double_linked_list_node *node);
void double_linked_list_remove(double_linked_list *list, double_linked_list_node *node);
void double_linked_list_print(double_linked_list *list);
void double_linked_list_printr(double_linked_list *list);

#endif
