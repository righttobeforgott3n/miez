#include "generic_queue.h"
#include "generic_linked_list.h"
#include <stdlib.h>

#define STDIO_DEBUG 1
#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct generic_queue_t
{
    generic_linked_list _generic_linked_list;
};

int
generic_queue_new(generic_queue* out_self)
{

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_queue_t* self =
        (struct generic_queue_t*) malloc(sizeof(struct generic_queue_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - allocation failed\n", __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    int exit_code = generic_linked_list_new(&(self->_generic_linked_list));
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - generic_linked_list_new failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        free(self);
        *out_self = NULL;

        return exit_code;
    }

    *out_self = self;

    return 0;
}

int
generic_queue_free(generic_queue self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    int exit_code = generic_linked_list_free(self->_generic_linked_list);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - generic_linked_list_free failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return exit_code;
    }

    free(self);

    return 0;
}

int
generic_queue_set_free_function(generic_queue self,
                                void (*free_function)(void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    return generic_linked_list_set_free_function(self->_generic_linked_list,
                                                 free_function);
}

int
generic_queue_get_free_function(generic_queue self,
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

    return generic_linked_list_get_free_function(self->_generic_linked_list,
                                                 out_free_function);
}

int
generic_queue_set_copy_function(generic_queue self,
                                int (*copy_function)(void*, void**))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    return generic_linked_list_set_copy_function(self->_generic_linked_list,
                                                 copy_function);
}

int
generic_queue_get_copy_function(generic_queue self,
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

    return generic_linked_list_get_copy_function(self->_generic_linked_list,
                                                 out_copy_function);
}

int
generic_queue_size(generic_queue self, size_t* out_size)
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

    return generic_linked_list_size(self->_generic_linked_list, out_size);
}

int
generic_queue_is_empty(generic_queue self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    return generic_linked_list_is_empty(self->_generic_linked_list);
}

int
generic_queue_enqueue(generic_queue self, void* data)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    return generic_linked_list_insert_last(self->_generic_linked_list, data);
}

int
generic_queue_dequeue(generic_queue self, void** out_data)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    return generic_linked_list_remove_first(self->_generic_linked_list,
                                            out_data);
}

int
generic_queue_peek(generic_queue self, void** out_data)
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

    if (generic_linked_list_is_empty(self->_generic_linked_list))
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - queue is empty\n", __PRETTY_FUNCTION__);
#endif

        *out_data = NULL;
        return 0;
    }

    generic_linked_list_iterator iter = NULL;
    int exit_code =
        generic_linked_list_iterator_begin(self->_generic_linked_list, &iter);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_begin failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return exit_code;
    }

    exit_code = generic_linked_list_iterator_get(iter, out_data);

    generic_linked_list_iterator_free(iter);

    return exit_code;
}

int
generic_queue_peek_rear(generic_queue self, void** out_data)
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

    if (generic_linked_list_is_empty(self->_generic_linked_list))
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - queue is empty\n", __PRETTY_FUNCTION__);
#endif

        *out_data = NULL;
        return 0;
    }

    generic_linked_list_iterator iter = NULL;
    int exit_code = generic_linked_list_iterator_reverse_begin(
        self->_generic_linked_list, &iter);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_reverse_begin failed with "
                "code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return exit_code;
    }

    exit_code = generic_linked_list_iterator_get(iter, out_data);

    generic_linked_list_iterator_free(iter);

    return exit_code;
}

int
generic_queue_clear(generic_queue self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    while (!generic_linked_list_is_empty(self->_generic_linked_list))
    {

        int exit_code =
            generic_linked_list_remove_first(self->_generic_linked_list, NULL);

        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr,
                    "%s - generic_linked_list_remove_first failed with code "
                    "%d\n",
                    __PRETTY_FUNCTION__, exit_code);
#endif

            return exit_code;
        }
    }

    return 0;
}

int
generic_queue_contains(generic_queue self, void* target,
                       int (*compare)(void*, void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    if (!compare)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - compare parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;
    generic_linked_list_iterator found = NULL;

    int exit_code =
        generic_linked_list_iterator_begin(self->_generic_linked_list, &begin);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_begin failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return -1;
    }

    exit_code =
        generic_linked_list_iterator_end(self->_generic_linked_list, &end);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_end failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        generic_linked_list_iterator_free(begin);
        return -1;
    }

    exit_code =
        generic_linked_list_iterator_find(begin, end, target, compare, &found);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_find failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        generic_linked_list_iterator_free(begin);
        generic_linked_list_iterator_free(end);
        return -1;
    }

    int result = generic_linked_list_iterator_is_valid(found) == 0 ? 1 : 0;

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_iterator_free(found);

    return result;
}

int
generic_queue_for_each(generic_queue self, void (*apply)(void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
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

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;

    int exit_code =
        generic_linked_list_iterator_begin(self->_generic_linked_list, &begin);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_begin failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return exit_code;
    }

    exit_code =
        generic_linked_list_iterator_end(self->_generic_linked_list, &end);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - generic_linked_list_iterator_end failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        generic_linked_list_iterator_free(begin);
        return exit_code;
    }

    exit_code = generic_linked_list_iterator_for_each(begin, end, apply);

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    return exit_code;
}
