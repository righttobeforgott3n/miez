#include "generic_queue.h"
#include "generic_linked_list.h"
#include <stdlib.h>

#define STDIO_DEBUG 1
#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct _generic_queue_meta_item_t
{
    void* _item;
    int (*_free_item_function)(void*);
    int (*_deep_item_function)(void*, void**);
};

struct generic_queue_t
{
    generic_linked_list _generic_linked_list;
};

// @todo initial_capacity is actually not supported.
int
generic_queue_new(size_t initial_capacity __attribute__((unused)),
                  generic_queue* out_self)
{

    if (!out_self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "out_self parameter equals NULL\n");
#endif

        return 1;
    }

    struct generic_queue_t* self =
        (struct generic_queue_t*) malloc(sizeof(struct generic_queue_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "struct generic_queue_t failed allocation\n");
#endif

        return -1;
    }

    int exit_code = generic_linked_list_new(&(self->_generic_linked_list));
    if (exit_code)
    {


#ifdef STDIO_DEBUG
        fprintf(stderr, "error from generic_linked_list_new - exit code: %d\n", exit_code);
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
        return 1;
    }

    int exit_code = generic_linked_list_free(self->_generic_linked_list);
    if (exit_code)
    {
        return exit_code;
    }

    free(self);

    return 0;
}
