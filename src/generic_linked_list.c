#include "generic_linked_list.h"
#include <stddef.h>
#include <stdlib.h>

struct node_t
{
    void* data;
    struct node_t* next;
    struct node_t* prev;
};

struct generic_linked_list_t
{
    size_t size;
    struct node_t* head_guard;
    struct node_t* tail_guard;
};

int
generic_linked_list_new(generic_linked_list* out_self)
{

    if (!out_self)
    {
        return 1;
    }

    *out_self =
        (generic_linked_list) malloc(sizeof(struct generic_linked_list_t));
    if (!*out_self)
    {
        return -1;
    }

    (*out_self)->size = 0;
    (*out_self)->head_guard = (struct node_t*) malloc(sizeof(struct node_t));
    (*out_self)->tail_guard = (struct node_t*) malloc(sizeof(struct node_t));
    if (!(*out_self)->head_guard || !(*out_self)->tail_guard)
    {

        free((*out_self)->head_guard);
        free((*out_self)->tail_guard);
        free(*out_self);

        return -1;
    }

    (*out_self)->head_guard->next = (*out_self)->tail_guard;
    (*out_self)->head_guard->prev = NULL;
    (*out_self)->tail_guard->prev = (*out_self)->head_guard;
    (*out_self)->tail_guard->next = NULL;

    return 0;
}

int
generic_linked_list_free(generic_linked_list self)
{

    if (!self)
    {
        return 1;
    }

    struct node_t* current = self->head_guard->next;
    while (current != self->tail_guard)
    {

        struct node_t* next = current->next;

        free(current);
        current = next;
    }

    free(self->head_guard);
    free(self->tail_guard);
    free(self);

    return 0;
}

int
generic_linked_list_size(generic_linked_list self, size_t* out_size)
{

    if (!self || !out_size)
    {
        return 1;
    }

    *out_size = self->size;

    return 0;
}

int
generic_linked_list_insert_first(generic_linked_list self, void* data)
{

    if (!self)
    {
        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {
        return -1;
    }

    new_node->data = data;
    new_node->next = self->head_guard->next;
    self->head_guard->next->prev = new_node;
    new_node->prev = self->head_guard;
    self->head_guard->next = new_node;
    self->size++;

    return 0;
}

int
generic_linked_list_insert_last(generic_linked_list self, void* data)
{

    if (!self)
    {
        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {
        return -1;
    }

    new_node->data = data;
    new_node->prev = self->tail_guard->prev;
    self->tail_guard->prev->next = new_node;
    new_node->next = self->tail_guard;
    self->tail_guard->prev = new_node;
    self->size++;

    return 0;
}

int
generic_linked_list_remove_first(generic_linked_list self, void** out_data)
{

    if (!self || !out_data)
    {
        return 1;
    }

    if (self->size == 0)
    {
        return 0;
    }

    struct node_t* to_remove = self->head_guard->next;
    *out_data = to_remove->data;
    self->head_guard->next = to_remove->next;
    to_remove->next->prev = self->head_guard;
    self->size--;

    free(to_remove);

    return 0;
}

int
generic_linked_list_remove_last(generic_linked_list self, void** out_data)
{

    if (!self || !out_data)
    {
        return 1;
    }

    if (self->size == 0)
    {
        return 0;
    }

    struct node_t* to_remove = self->tail_guard->prev;
    *out_data = to_remove->data;
    self->tail_guard->prev = to_remove->prev;
    to_remove->prev->next = self->tail_guard;
    self->size--;

    free(to_remove);

    return 0;
}