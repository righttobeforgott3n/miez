#include "generic_queue.h"
#include "generic_linked_list.h"
#include <stdlib.h>

struct generic_queue_t
{
    generic_linked_list list;
};

generic_queue
generic_queue_new()
{

    generic_queue queue =
        (generic_queue) malloc(sizeof(struct generic_queue_t));
    if (!queue)
    {
        return NULL;
    }

    queue->list = generic_linked_list_new();
    if (!queue->list)
    {
        free(queue);
        return NULL;
    }

    return queue;
}

void
generic_queue_free(generic_queue queue)
{

    if (!queue)
    {
        return;
    }

    generic_linked_list_free(queue->list);
    free(queue);
}

size_t
generic_queue_size(generic_queue queue)
{

    if (!queue)
    {
        return 0;
    }

    return generic_linked_list_size(queue->list);
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
generic_queue_dequeue(generic_queue queue, void** data)
{

    if (!queue || !data)
    {
        return 1;
    }

    return generic_linked_list_remove_first(queue->list, data);
}
