#include "pseudo_generic_queue.h"
#include "pseudo_generic_linked_list.h"
#include <stdlib.h>

struct pseudo_generic_queue_t
{
    pseudo_generic_linked_list list;
};

pseudo_generic_queue
pseudo_generic_queue_new()
{

    pseudo_generic_queue queue =
        (pseudo_generic_queue) malloc(sizeof(struct pseudo_generic_queue_t));
    if (!queue)
    {
        return NULL;
    }

    queue->list = pseudo_generic_linked_list_new();
    if (!queue->list)
    {
        free(queue);
        return NULL;
    }

    return queue;
}

void
pseudo_generic_queue_free(pseudo_generic_queue queue)
{

    if (!queue)
    {
        return;
    }

    pseudo_generic_linked_list_free(queue->list);
    free(queue);
}

size_t
pseudo_generic_queue_size(pseudo_generic_queue queue)
{

    if (!queue)
    {
        return 0;
    }

    return pseudo_generic_linked_list_size(queue->list);
}

int
pseudo_generic_queue_enqueue(pseudo_generic_queue queue, void* data)
{

    if (!queue)
    {
        return 1;
    }

    return pseudo_generic_linked_list_insert_last(queue->list, data);
}

int
pseudo_generic_queue_dequeue(pseudo_generic_queue queue, void** data)
{

    if (!queue || !data)
    {
        return 1;
    }

    return pseudo_generic_linked_list_remove_first(queue->list, data);
}
