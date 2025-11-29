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
    size_t _size;
    struct node_t* _head_guard;
    struct node_t* _tail_guard;
    void (*_free_function)(void*);
    int (*_copy_function)(void*, void**);
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

    (*out_self)->_size = 0;
    (*out_self)->_free_function = NULL;
    (*out_self)->_copy_function = NULL;
    (*out_self)->_head_guard = (struct node_t*) malloc(sizeof(struct node_t));
    (*out_self)->_tail_guard = (struct node_t*) malloc(sizeof(struct node_t));

    if (!(*out_self)->_head_guard || !(*out_self)->_tail_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for guard nodes\n",
                __PRETTY_FUNCTION__);
#endif

        free((*out_self)->_head_guard);
        free((*out_self)->_tail_guard);
        free(*out_self);

        *out_self = NULL;

        return -1;
    }

    (*out_self)->_head_guard->data = NULL;
    (*out_self)->_head_guard->next = (*out_self)->_tail_guard;
    (*out_self)->_head_guard->prev = NULL;
    (*out_self)->_tail_guard->data = NULL;
    (*out_self)->_tail_guard->prev = (*out_self)->_head_guard;
    (*out_self)->_tail_guard->next = NULL;

    return 0;
}

int
generic_linked_list_set_free_function(generic_linked_list self,
                                      void (*free_function)(void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    self->_free_function = free_function;

    return 0;
}

int
generic_linked_list_get_free_function(generic_linked_list self,
                                      void (**out_free_function)(void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_free_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_free_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_free_function = self->_free_function;

    return 0;
}

int
generic_linked_list_set_copy_function(generic_linked_list self,
                                      int (*copy_function)(void*, void**))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    self->_copy_function = copy_function;

    return 0;
}

int
generic_linked_list_get_copy_function(generic_linked_list self,
                                      int (**out_copy_function)(void*, void**))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_copy_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_copy_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_copy_function = self->_copy_function;

    return 0;
}

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

    if (self->_head_guard && self->_tail_guard)
    {

        struct node_t* current = self->_head_guard->next;
        while (current != self->_tail_guard)
        {

            struct node_t* next = current->next;

            if (self->_free_function && current->data)
            {
                self->_free_function(current->data);
            }

            free(current);
            current = next;
        }
    }

    free(self->_head_guard);
    free(self->_tail_guard);
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

    *out_size = self->_size;

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

    if (self->_copy_function)
    {

        void* copied_data = NULL;
        int copy_result = self->_copy_function(data, &copied_data);
        if (copy_result != 0)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - _copy_function failed with code %d\n",
                    __PRETTY_FUNCTION__, copy_result);
#endif

            free(new_node);
            return copy_result;
        }

        new_node->data = copied_data;
    }
    else
    {

        new_node->data = data;
    }

    new_node->next = self->_head_guard->next;
    self->_head_guard->next->prev = new_node;
    new_node->prev = self->_head_guard;
    self->_head_guard->next = new_node;
    self->_size++;

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

    if (self->_copy_function)
    {

        void* copied_data = NULL;
        int copy_result = self->_copy_function(data, &copied_data);
        if (copy_result != 0)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - _copy_function failed with code %d\n",
                    __PRETTY_FUNCTION__, copy_result);
#endif

            free(new_node);
            return copy_result;
        }

        new_node->data = copied_data;
    }
    else
    {

        new_node->data = data;
    }

    new_node->prev = self->_tail_guard->prev;
    self->_tail_guard->prev->next = new_node;
    new_node->next = self->_tail_guard;
    self->_tail_guard->prev = new_node;
    self->_size++;

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

    if (self->_size == 0)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - list is empty\n", __PRETTY_FUNCTION__);
#endif

        if (out_data)
        {
            *out_data = NULL;
        }

        return 0;
    }

    if (!out_data && !self->_free_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data is NULL and _free_function is not set\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* to_remove = self->_head_guard->next;

    if (!out_data && self->_free_function)
    {

        if (to_remove->data)
        {
            self->_free_function(to_remove->data);
        }
    }
    else if (out_data)
    {

        *out_data = to_remove->data;
    }

    self->_head_guard->next = to_remove->next;
    to_remove->next->prev = self->_head_guard;
    self->_size--;

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

    if (self->_size == 0)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - list is empty\n", __PRETTY_FUNCTION__);
#endif

        if (out_data)
        {
            *out_data = NULL;
        }

        return 0;
    }

    if (!out_data && !self->_free_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data is NULL and _free_function is not set\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* to_remove = self->_tail_guard->prev;

    if (!out_data && self->_free_function)
    {

        if (to_remove->data)
        {
            self->_free_function(to_remove->data);
        }
    }
    else if (out_data)
    {

        *out_data = to_remove->data;
    }

    self->_tail_guard->prev = to_remove->prev;
    to_remove->prev->next = self->_tail_guard;
    self->_size--;

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

    if (index >= self->_size)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - index %zu out of bounds (_size: %zu)\n",
                __PRETTY_FUNCTION__, index, self->_size);
#endif

        return 1;
    }

    if (!out_data && !self->_free_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data is NULL and _free_function is not set\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* to_delete = NULL;
    size_t mid = self->_size / 2;

    if (index <= mid)
    {

        to_delete = self->_head_guard->next;
        for (size_t i = 0; i < index; i++)
        {
            to_delete = to_delete->next;
        }
    }
    else
    {

        to_delete = self->_tail_guard->prev;
        for (size_t i = self->_size - 1; i > index; i--)
        {
            to_delete = to_delete->prev;
        }
    }

    if (to_delete == self->_tail_guard || to_delete == self->_head_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - reached guard node\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_data && self->_free_function)
    {

        if (to_delete->data)
        {
            self->_free_function(to_delete->data);
        }
    }
    else if (out_data)
    {

        *out_data = to_delete->data;
    }

    to_delete->prev->next = to_delete->next;
    to_delete->next->prev = to_delete->prev;

    free(to_delete);
    self->_size--;

    return 0;
}
