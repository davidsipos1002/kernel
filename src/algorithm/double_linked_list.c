#include <algorithm/double_linked_list.h>

void double_linked_list_init(double_linked_list *list)
{
    list->head = 0;
    list->length = 0;
}

void double_linked_list_insert_front(double_linked_list *list, double_linked_list_node *node)
{
    list->length++;
    if (!list->head)
    {
        node->prev = node;
        node->next = node;
        list->head = node;
        return;
    }
    node->prev = list->head->prev;
    node->next = list->head;
    list->head->prev->next = node;
    list->head->prev = node;
    list->head = node;
}

double_linked_list_node *double_linked_list_remove_front(double_linked_list *list)
{
    if (!list->length)
        return 0;

    double_linked_list_node *ret = list->head;
    list->length--;
    if (!list->length)
    {
        list->head = 0;
        ret->prev = 0;
        ret->next = 0;
        return ret;
    }
    double_linked_list_node *snd = list->head->next;
    snd->prev = list->head->prev;
    list->head->prev->next = snd;
    list->head = snd;
    ret->prev = 0;
    ret->next = 0;
    return ret;
}

void double_linked_list_insert_back(double_linked_list *list, double_linked_list_node *node)
{
    list->length++;
    if (!list->head)
    {
        node->prev = node;
        node->next = node;
        list->head = node;
        return;
    }
    double_linked_list_node *lst = list->head->prev;
    node->next = list->head;
    node->prev = lst;
    list->head->prev = node;
    lst->next = node;
}

double_linked_list_node *double_linked_list_remove_back(double_linked_list *list)
{
    if (!list->length)
        return 0;

    list->length--;
    double_linked_list_node *ret = list->head->prev;
    if (!list->length)
    {
        list->head = 0;
        ret->prev = 0;
        ret->next = 0;
        return ret;
    }
    ret->prev->next = list->head;
    list->head->prev = ret->prev;
    ret->prev = 0;
    ret->next = 0;
    return ret;
}

void double_linked_list_insert_after(double_linked_list *list, double_linked_list_node *after, double_linked_list_node *node)
{
    if (!list->length)
        return;

    if (list->head->prev == after)
    {
        double_linked_list_insert_back(list, node);
        return;
    }
    list->length++;
    node->prev = after;
    node->next = after->next;
    after->next->prev = node;
    after->next = node;
}

void double_linked_list_remove(double_linked_list *list, double_linked_list_node *node)
{
    if (!list->length)
        return;

    if (list->head == node)
    {
        double_linked_list_remove_front(list);
        return;
    }
    list->length--;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->prev = 0;
    node->next = 0;
}