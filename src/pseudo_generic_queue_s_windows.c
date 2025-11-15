#include "pseudo_generic_queue.h"
#include "pseudo_generic_queue_s.h"
#include <stdlib.h>
#include <windows.h>

struct pseudo_generic_queue_s_t
{
    pseudo_generic_queue queue;
    CRITICAL_SECTION
        critical_section;
};

pseudo_generic_queue_s
pseudo_generic_queue_s_new()
{

    pseudo_generic_queue_s queue_s = (pseudo_generic_queue_s) malloc(
        sizeof(struct pseudo_generic_queue_s_t));

    if (!queue_s)
    {
        return NULL;
    }

    queue_s->queue = pseudo_generic_queue_new();
    if (!queue_s->queue)
    {
        free(queue_s);
        return NULL;
    }

    InitializeCriticalSection(&queue_s->critical_section);

    return queue_s;
}

void
pseudo_generic_queue_s_free(pseudo_generic_queue_s queue)
{

    if (!queue)
    {
        return;
    }

    DeleteCriticalSection(&queue->critical_section);
    pseudo_generic_queue_free(queue->queue);

    free(queue);
}

size_t
pseudo_generic_queue_s_size(pseudo_generic_queue_s queue)
{

    if (!queue)
    {
        return 0;
    }

    EnterCriticalSection(&queue->critical_section);
    size_t size = pseudo_generic_queue_size(queue->queue);
    LeaveCriticalSection(&queue->critical_section);

    return size;
}

int
pseudo_generic_queue_s_enqueue(pseudo_generic_queue_s queue, void* data)
{
    if (!queue)
    {
        return 1;
    }


    EnterCriticalSection(&queue->critical_section);
    int result = pseudo_generic_queue_enqueue(queue->queue, data);
    LeaveCriticalSection(&queue->critical_section);

    return result;
}

int
pseudo_generic_queue_s_dequeue(pseudo_generic_queue_s queue, void** data)
{

    if (!queue || !data)
    {
        return 1;
    }

    EnterCriticalSection(&queue->critical_section);
    int result = pseudo_generic_queue_dequeue(queue->queue, data);
    LeaveCriticalSection(&queue->critical_section);

    return result;
}
