#include "generic_queue.h"
#include "generic_linked_list.h"
#include <stdlib.h>

struct generic_queue_t
{
    generic_linked_list list;
};

int
generic_queue_new(generic_queue* out_self)
{

    if (!out_self)
    {
        return 1;
    }

    *out_self = (generic_queue) malloc(sizeof(struct generic_queue_t));
    if (!*out_self)
    {
        return -1;
    }

    int result = generic_linked_list_new(&((*out_self)->list));
    if (result && !(*out_self)->list)
    {

        free(*out_self);
        *out_self = NULL;

        return -1;
    }

    return 0;
}

int
generic_queue_free(generic_queue self)
{

    if (!self)
    {
        return 1;
    }

    if (self->list)
    {
        generic_linked_list_free(self->list);
    }
    free(self);

    return 0;
}

int
generic_queue_size(generic_queue queue, size_t* out_size)
{

    if (!queue || !out_size)
    {
        return 1;
    }

    generic_linked_list_size(queue->list, out_size);

    return 0;
}

int
generic_queue_enqueue(generic_queue queue, void* data)
{

    if (!queue)
    {
        return 1;
    }

    return generic_linked_list_insert_last(queue->list, data);
}

int
generic_queue_dequeue(generic_queue queue, void** out_data)
{

    if (!queue || !out_data)
    {
        return 1;
    }

    return generic_linked_list_remove_first(queue->list, out_data);
}
