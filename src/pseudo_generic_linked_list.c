#include "pseudo_generic_linked_list.h"
#include <stddef.h>
#include <stdlib.h>

struct node_t
{
    void* data;
    struct node_t* next;
    struct node_t* prev;
};

struct pseudo_generic_linked_list_t
{
    size_t size;
    struct node_t* head_guard;
    struct node_t* tail_guard;
};

pseudo_generic_linked_list
pseudo_generic_linked_list_new()
{

    pseudo_generic_linked_list list = (pseudo_generic_linked_list) malloc(
        sizeof(struct pseudo_generic_linked_list_t));
    if (!list)
    {
        return NULL;
    }

    list->size = 0;
    list->head_guard = (struct node_t*) malloc(sizeof(struct node_t));
    list->tail_guard = (struct node_t*) malloc(sizeof(struct node_t));
    if (!list->head_guard || !list->tail_guard)
    {

        free(list->head_guard);
        free(list->tail_guard);
        free(list);

        return NULL;
    }

    list->head_guard->next = list->tail_guard;
    list->head_guard->prev = NULL;
    list->tail_guard->prev = list->head_guard;
    list->tail_guard->next = NULL;

    return list;
}

void
pseudo_generic_linked_list_free(pseudo_generic_linked_list list)
{

    if (!list)
    {
        return;
    }

    struct node_t* current = list->head_guard->next;
    while (current != list->tail_guard)
    {

        struct node_t* next = current->next;

        free(current);
        current = next;
    }

    free(list->head_guard);
    free(list->tail_guard);
    free(list);
}

size_t
pseudo_generic_linked_list_size(pseudo_generic_linked_list list)
{

    if (!list)
    {
        return 0;
    }

    return list->size;
}

int
pseudo_generic_linked_list_insert_first(pseudo_generic_linked_list list,
                                        void* data)
{

    if (!list)
    {
        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {
        return 2;
    }

    new_node->data = data;
    new_node->next = list->head_guard->next;
    list->head_guard->next->prev = new_node;
    new_node->prev = list->head_guard;
    list->head_guard->next = new_node;
    list->size++;

    return 0;
}

int
pseudo_generic_linked_list_insert_last(pseudo_generic_linked_list list,
                                       void* data)
{

    if (!list)
    {
        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {
        return 2;
    }

    new_node->data = data;
    new_node->prev = list->tail_guard->prev;
    list->tail_guard->prev->next = new_node;
    new_node->next = list->tail_guard;
    list->tail_guard->prev = new_node;
    list->size++;

    return 0;
}

int
pseudo_generic_linked_list_remove_first(pseudo_generic_linked_list list,
                                        void** data)
{

    if (!list || !data)
    {
        return 1;
    }

    if (list->size == 0)
    {
        return 0;
    }

    struct node_t* to_remove = list->head_guard->next;
    *data = to_remove->data;
    list->head_guard->next = to_remove->next;
    to_remove->next->prev = list->head_guard;
    list->size--;

    free(to_remove);

    return 0;
}

int
pseudo_generic_linked_list_remove_last(pseudo_generic_linked_list list,
                                       void** data)
{

    if (!list || !data)
    {
        return 1;
    }

    if (list->size == 0)
    {
        return 0;
    }

    struct node_t* to_remove = list->tail_guard->prev;
    *data = to_remove->data;
    list->tail_guard->prev = to_remove->prev;
    to_remove->prev->next = list->tail_guard;
    list->size--;

    free(to_remove);

    return 0;
}