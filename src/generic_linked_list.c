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

struct generic_linked_list_iterator_t
{
    generic_linked_list _list;
    struct node_t* _current_node;
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
generic_linked_list_is_empty(generic_linked_list self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    return self->_size == 0 ? 1 : 0;
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
generic_linked_list_iterator_begin(generic_linked_list ll,
                                   generic_linked_list_iterator* out_self)
{

    if (!ll)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - ll parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_linked_list_iterator_t* self =
        (struct generic_linked_list_iterator_t*) malloc(
            sizeof(struct generic_linked_list_iterator_t));

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    self->_list = ll;
    self->_current_node = ll->_head_guard->next;
    *out_self = self;

    return 0;
}

int
generic_linked_list_iterator_end(generic_linked_list ll,
                                 generic_linked_list_iterator* out_self)
{

    if (!ll)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - ll parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_linked_list_iterator_t* self =
        (struct generic_linked_list_iterator_t*) malloc(
            sizeof(struct generic_linked_list_iterator_t));

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    self->_list = ll;
    self->_current_node = ll->_tail_guard;
    *out_self = self;

    return 0;
}

int
generic_linked_list_iterator_reverse_begin(
    generic_linked_list ll, generic_linked_list_iterator* out_self)
{

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!ll)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - ll parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_linked_list_iterator_t* self =
        (struct generic_linked_list_iterator_t*) malloc(
            sizeof(struct generic_linked_list_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    self->_list = ll;
    self->_current_node = ll->_tail_guard->prev;
    *out_self = self;

    return 0;
}

int
generic_linked_list_iterator_reverse_end(generic_linked_list ll,
                                         generic_linked_list_iterator* out_self)
{

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!ll)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - ll parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_linked_list_iterator_t* self =
        (struct generic_linked_list_iterator_t*) malloc(
            sizeof(struct generic_linked_list_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    self->_list = ll;
    self->_current_node = ll->_head_guard;
    *out_self = self;

    return 0;
}

int
generic_linked_list_iterator_free(generic_linked_list_iterator self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    free(self);

    return 0;
}

int
generic_linked_list_iterator_get(generic_linked_list_iterator self,
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

    if (self->_current_node == self->_list->_head_guard
        || self->_current_node == self->_list->_tail_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - iterator at guard node\n", __PRETTY_FUNCTION__);
#endif

        *out_data = NULL;
        return 1;
    }

    *out_data = self->_current_node->data;

    return 0;
}

int
generic_linked_list_iterator_next(generic_linked_list_iterator self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->_current_node == self->_list->_tail_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - already at end\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    self->_current_node = self->_current_node->next;

    return 0;
}

int
generic_linked_list_iterator_prev(generic_linked_list_iterator self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->_current_node == self->_list->_head_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - already at rend\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    self->_current_node = self->_current_node->prev;

    return 0;
}

int
generic_linked_list_iterator_is_end(generic_linked_list_iterator self)
{

    if (!self)
    {
        return 1;
    }

    return !(self->_current_node == self->_list->_tail_guard);
}

int
generic_linked_list_iterator_is_begin(generic_linked_list_iterator self)
{

    if (!self)
    {
        return 1;
    }

    return !(self->_current_node == self->_list->_head_guard);
}

int
generic_linked_list_iterator_is_valid(generic_linked_list_iterator self)
{

    if (!self)
    {
        return 1;
    }

    if (self->_current_node == self->_list->_head_guard
        || self->_current_node == self->_list->_tail_guard)
    {
        return 1;
    }

    return 0;
}

int
generic_linked_list_iterator_advance(generic_linked_list_iterator self,
                                     size_t n)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    size_t i = 0;
    while (i < n && self->_current_node != self->_list->_tail_guard)
    {
        self->_current_node = self->_current_node->next;
        i++;
    }

    return 0;
}

int
generic_linked_list_iterator_reverse_advance(generic_linked_list_iterator self,
                                             size_t n)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    size_t i = 0;
    while (i < n && self->_current_node != self->_list->_head_guard)
    {
        self->_current_node = self->_current_node->prev;
        i++;
    }

    return 0;
}

int
generic_linked_list_iterator_remove(generic_linked_list_iterator self,
                                    void** out_data)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->_current_node == self->_list->_head_guard
        || self->_current_node == self->_list->_tail_guard)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - iterator at guard node\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_data && !self->_list->_free_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_data is NULL and _free_function is not set\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* to_remove = self->_current_node;
    struct node_t* next_node = to_remove->next;

    // @todo improve this if-else structure: I would like to make it atomic as
    // possible.
    if (!out_data && self->_list->_free_function)
    {

        if (to_remove->data)
        {
            self->_list->_free_function(to_remove->data);
        }
    }
    else if (out_data)
    {
        *out_data = to_remove->data;
    }

    to_remove->prev->next = to_remove->next;
    to_remove->next->prev = to_remove->prev;

    free(to_remove);
    self->_list->_size--;
    self->_current_node = next_node;

    return 0;
}

int
generic_linked_list_iterator_distance(generic_linked_list_iterator begin,
                                      generic_linked_list_iterator end,
                                      size_t* out_distance)
{

    if (!begin)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - begin parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!end)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - end parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_distance)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_distance parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (begin->_list != end->_list)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - iterators from different lists\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    size_t distance = 0;
    struct node_t* current = begin->_current_node;
    while (current && current != end->_current_node)
    {

        if (current == begin->_list->_tail_guard)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - end not reachable from begin\n",
                    __PRETTY_FUNCTION__);
#endif

            return 1;
        }

        if (current != begin->_list->_head_guard)
        {
            distance++;
        }

        current = current->next;
    }

    *out_distance = distance;

    return 0;
}

int
generic_linked_list_iterator_find(generic_linked_list_iterator begin,
                                  generic_linked_list_iterator end,
                                  void* target, int (*compare)(void*, void*),
                                  generic_linked_list_iterator* out_found)
{

    if (!begin)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - begin parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!end)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - end parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!compare)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - compare parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_found)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_found parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (begin->_list != end->_list)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - iterators from different lists\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* current = begin->_current_node;
    while (current && current != end->_current_node)
    {

        if (current == begin->_list->_head_guard
            || current == begin->_list->_tail_guard)
        {
            current = current->next;
            continue;
        }

        if (compare(current->data, target) == 0)
        {

            struct generic_linked_list_iterator_t* found =
                (struct generic_linked_list_iterator_t*) malloc(
                    sizeof(struct generic_linked_list_iterator_t));

            if (!found)
            {

#ifdef STDIO_DEBUG
                fprintf(stderr, "%s - allocation failed for iterator\n",
                        __PRETTY_FUNCTION__);
#endif

                return -1;
            }

            found->_list = begin->_list;
            found->_current_node = current;
            *out_found = found;

            return 0;
        }

        current = current->next;
    }

    struct generic_linked_list_iterator_t* not_found =
        (struct generic_linked_list_iterator_t*) malloc(
            sizeof(struct generic_linked_list_iterator_t));

    if (!not_found)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed for iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    not_found->_list = end->_list;
    not_found->_current_node = end->_current_node;
    *out_found = not_found;

    return 0;
}

int
generic_linked_list_iterator_for_each(generic_linked_list_iterator begin,
                                      generic_linked_list_iterator end,
                                      void (*apply)(void*))
{

    if (!begin)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - begin parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!end)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - end parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!apply)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - apply parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (begin->_list != end->_list)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - iterators from different lists\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct node_t* current = begin->_current_node;
    while (current && current != end->_current_node
           && current != end->_list->_tail_guard)
    {
        apply(current->data);
        current = current->next;
    }

    return 0;
}

// @todo move the generic_linked_list related modules into a separate repo and
// include it through git sub-module.