#include <algorithm/linked_list.h>

void linked_list_init(linked_list *list)
{
    list->head = 0;
    list->last = 0;
    list->count = 0;
}

void linked_list_insert_front(linked_list *list, linked_list_node *node)
{
    list->count++;
    if(list->head == 0)
    {
        node->link = 0;
        list->head = list->last = (uint64_t) node;
        return;
    }
    node->link = list->head;
    ((linked_list_node *)list->head)->link ^= (uint64_t) node;
    list->head = (uint64_t) node;
}

void linked_list_insert_rear(linked_list *list, linked_list_node *node)
{
    list->count++;
    if(list->head == 0)
    {
        node->link = 0;
        list->head = list->last = (uint64_t) node;
        return;
    }
    node->link = list->last;
    ((linked_list_node *)list->last)->link ^= (uint64_t) node;
    list->last = (uint64_t) node;
}

linked_list_node* linked_list_find(linked_list *list, uint64_t key)
{
    uint64_t prev = 0;
    uint64_t curr = list->head;
    uint64_t aux = 0;
    while(curr != 0)
    {
        if(key == ((linked_list_node *) curr)->key)
            return (linked_list_node *) curr;
        aux = curr;
        curr = ((linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    return (linked_list_node *) 0;
}

void linked_list_insert_after(linked_list *list, uint64_t after, linked_list_node *node) 
{
    uint64_t prev = 0;
    uint64_t curr = list->head;
    uint64_t aux = 0;
    while(curr != 0 && ((linked_list_node *) curr)->key != after)
    {
        aux = curr;
        curr = ((linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    if(curr == 0)
        return;
    else if(curr == list->last)
        linked_list_insert_rear(list, node);
    else
    {
        aux = prev ^ ((linked_list_node *) curr)->link;
        node->link = curr ^ aux;
        ((linked_list_node *) aux)->link ^= curr ^ (uint64_t) node;
        ((linked_list_node *) curr)->link ^= aux ^ (uint64_t) node;
        list->count++;
    }
}

void linked_list_delete_first(linked_list *list)
{
    if(list->count == 0)
        return;
    list->count--;
    if(((linked_list_node *) list->head)->link == 0)
    {
        list->head = list->last = 0;
        return;
    }
    uint64_t del = list->head;
    uint64_t next = ((linked_list_node *) list->head)->link;
    ((linked_list_node *) next)->link ^= del;
    list->head = next;
}

void linked_list_delete_last(linked_list *list)
{
    if(list->count == 0)
        return;
    list->count--;
    if(((linked_list_node *) list->head)->link == 0)
    {
        list->head = list->last = 0;
        return;
    }
    uintptr_t del = list->last;
    uintptr_t prev = ((linked_list_node *) list->last)->link;
    ((linked_list_node *) prev)->link ^= del;
    list->last = prev;
}

uint8_t linked_list_delete(linked_list *list, uint64_t key)
{
    uintptr_t prev = 0;
    uintptr_t curr = list->head;
    uintptr_t aux = 0;
    while(curr != 0 && ((linked_list_node *) curr)->key != key)
    {
        aux = curr;
        curr = ((linked_list_node *) curr)->link ^ prev;
        prev = aux;
    }
    if(curr == 0)
        return 0;
    else if(curr == list->head)
        linked_list_delete_first(list);
    else if(curr == list->last)
        linked_list_delete_last(list);
    else
    {
        aux = prev ^ ((linked_list_node *) curr)->link;
        ((linked_list_node *) prev)->link ^= curr ^ aux;
        ((linked_list_node *) aux)->link ^= curr ^ prev;
        list->count--;
    }
    return 1;
}