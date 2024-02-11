#include <algorithm/xor_linked_list.h>

void xor_linked_list_init(xor_linked_list *list)
{
    list->head = 0;
    list->last = 0;
    list->count = 0;
}

void xor_linked_list_insert_front(xor_linked_list *list, xor_linked_list_node *node)
{
    list->count++;
    if(list->head == 0)
    {
        node->link = 0;
        list->head = list->last = (uint64_t) node;
        return;
    }
    node->link = list->head;
    ((xor_linked_list_node *)list->head)->link ^= (uint64_t) node;
    list->head = (uint64_t) node;
}

void xor_linked_list_insert_rear(xor_linked_list *list, xor_linked_list_node *node)
{
    list->count++;
    if(list->head == 0)
    {
        node->link = 0;
        list->head = list->last = (uint64_t) node;
        return;
    }
    node->link = list->last;
    ((xor_linked_list_node *)list->last)->link ^= (uint64_t) node;
    list->last = (uint64_t) node;
}

xor_linked_list_node* xor_linked_list_find(xor_linked_list *list, uint64_t key)
{
    uint64_t prev = 0;
    uint64_t curr = list->head;
    uint64_t aux = 0;
    while(curr != 0)
    {
        if(key == ((xor_linked_list_node *) curr)->key)
            return (xor_linked_list_node *) curr;
        aux = curr;
        curr = ((xor_linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    return (xor_linked_list_node *) 0;
}

void xor_linked_list_insert_after(xor_linked_list *list, uint64_t after, xor_linked_list_node *node) 
{
    uint64_t prev = 0;
    uint64_t curr = list->head;
    uint64_t aux = 0;
    while(curr != 0 && ((xor_linked_list_node *) curr)->key != after)
    {
        aux = curr;
        curr = ((xor_linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    if(curr == 0)
        return;
    else if(curr == list->last)
        xor_linked_list_insert_rear(list, node);
    else
    {
        aux = prev ^ ((xor_linked_list_node *) curr)->link;
        node->link = curr ^ aux;
        ((xor_linked_list_node *) aux)->link ^= curr ^ (uint64_t) node;
        ((xor_linked_list_node *) curr)->link ^= aux ^ (uint64_t) node;
        list->count++;
    }
}

void xor_linked_list_delete_first(xor_linked_list *list)
{
    if(list->count == 0)
        return;
    list->count--;
    if(((xor_linked_list_node *) list->head)->link == 0)
    {
        list->head = list->last = 0;
        return;
    }
    uint64_t del = list->head;
    uint64_t next = ((xor_linked_list_node *) list->head)->link;
    ((xor_linked_list_node *) next)->link ^= del;
    list->head = next;
}

void xor_linked_list_delete_last(xor_linked_list *list)
{
    if(list->count == 0)
        return;
    list->count--;
    if(((xor_linked_list_node *) list->head)->link == 0)
    {
        list->head = list->last = 0;
        return;
    }
    uintptr_t del = list->last;
    uintptr_t prev = ((xor_linked_list_node *) list->last)->link;
    ((xor_linked_list_node *) prev)->link ^= del;
    list->last = prev;
}

uint8_t xor_linked_list_delete(xor_linked_list *list, uint64_t key)
{
    uintptr_t prev = 0;
    uintptr_t curr = list->head;
    uintptr_t aux = 0;
    while(curr != 0 && ((xor_linked_list_node *) curr)->key != key)
    {
        aux = curr;
        curr = ((xor_linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    if(curr == 0)
        return 0;
    else if(curr == list->head)
        xor_linked_list_delete_first(list);
    else if(curr == list->last)
        xor_linked_list_delete_last(list);
    else
    {
        aux = prev ^ ((xor_linked_list_node *) curr)->link;
        ((xor_linked_list_node *) prev)->link ^= curr ^ aux;
        ((xor_linked_list_node *) aux)->link ^= curr ^ prev;
        list->count--;
    }
    return 1;
}