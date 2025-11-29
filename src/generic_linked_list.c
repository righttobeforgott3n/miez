#include "generic_linked_list.h"
#include <stddef.h>
#include <stdlib.h>

#define STDIO_DEBUG
#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

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

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_self =
        (generic_linked_list) malloc(sizeof(struct generic_linked_list_t));
    if (!*out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for generic_linked_list_t\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    (*out_self)->size = 0;
    (*out_self)->head_guard = (struct node_t*) malloc(sizeof(struct node_t));
    (*out_self)->tail_guard = (struct node_t*) malloc(sizeof(struct node_t));

    if (!(*out_self)->head_guard || !(*out_self)->tail_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for guard nodes\n",
                __PRETTY_FUNCTION__);
#endif

        free((*out_self)->head_guard);
        free((*out_self)->tail_guard);
        free(*out_self);

        *out_self = NULL;

        return -1;
    }

    (*out_self)->head_guard->data = NULL;
    (*out_self)->head_guard->next = (*out_self)->tail_guard;
    (*out_self)->head_guard->prev = NULL;
    (*out_self)->tail_guard->data = NULL;
    (*out_self)->tail_guard->prev = (*out_self)->head_guard;
    (*out_self)->tail_guard->next = NULL;

    return 0;
}

// @todo adjust the free function: return items pointers or not?
int
generic_linked_list_free(generic_linked_list self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->head_guard && self->tail_guard)
    {

        struct node_t* current = self->head_guard->next;
        while (current != self->tail_guard)
        {
            struct node_t* next = current->next;
            free(current);
            current = next;
        }
    }

    free(self->head_guard);
    free(self->tail_guard);
    free(self);

    return 0;
}

int
generic_linked_list_size(generic_linked_list self, size_t* out_size)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_size)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_size parameter NULL\n", __PRETTY_FUNCTION__);
#endif

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

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for new node\n",
                __PRETTY_FUNCTION__);
#endif

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

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* new_node = (struct node_t*) malloc(sizeof(struct node_t));
    if (!new_node)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for new node\n",
                __PRETTY_FUNCTION__);
#endif

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

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_data)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->size == 0)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - list is empty\n", __PRETTY_FUNCTION__);
#endif

        *out_data = NULL;
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

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_data)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->size == 0)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - list is empty\n", __PRETTY_FUNCTION__);
#endif

        *out_data = NULL;
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

int
generic_linked_list_remove(generic_linked_list self, size_t index,
                           void** out_data)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_data)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (index >= self->size)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - index %zu out of bounds (size: %zu)\n",
                __PRETTY_FUNCTION__, index, self->size);
#endif

        return 1;
    }

    struct node_t* to_delete = NULL;
    size_t mid = self->size / 2;

    if (index <= mid)
    {

        to_delete = self->head_guard->next;
        for (size_t i = 0; i < index; i++)
        {
            to_delete = to_delete->next;
        }
    }
    else
    {

        to_delete = self->tail_guard->prev;
        for (size_t i = self->size - 1; i > index; i--)
        {
            to_delete = to_delete->prev;
        }
    }

    if (to_delete == self->tail_guard || to_delete == self->head_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - reached guard node\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_data = to_delete->data;

    to_delete->prev->next = to_delete->next;
    to_delete->next->prev = to_delete->prev;

    free(to_delete);
    self->size--;

    return 0;
}