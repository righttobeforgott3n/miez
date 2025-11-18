#include "generic_queue.h"
#include "generic_queue_s.h"
#include <stdlib.h>
#include <windows.h>

struct generic_queue_s_t
{
    generic_queue _generic_queue;
    CRITICAL_SECTION
    critical_section;
};

generic_queue_s
generic_queue_s_new()
{

    generic_queue_s queue_s =
        (generic_queue_s) malloc(sizeof(struct generic_queue_s_t));

    if (!queue_s)
    {
        return NULL;
    }

    queue_s->_generic_queue = generic_queue_new();
    if (!queue_s->_generic_queue)
    {
        free(queue_s);
        return NULL;
    }

    InitializeCriticalSection(&queue_s->critical_section);

    return queue_s;
}

void
generic_queue_s_free(generic_queue_s queue)
{

    if (!queue)
    {
        return;
    }

    DeleteCriticalSection(&queue->critical_section);
    generic_queue_free(queue->_generic_queue);

    free(queue);
}

size_t
generic_queue_s_size(generic_queue_s queue)
{

    if (!queue)
    {
        return 0;
    }

    EnterCriticalSection(&queue->critical_section);
    size_t size = generic_queue_size(queue->_generic_queue);
    LeaveCriticalSection(&queue->critical_section);

    return size;
}

int
generic_queue_s_enqueue(generic_queue_s queue, void* data)
{
    if (!queue)
    {
        return 1;
    }

    EnterCriticalSection(&queue->critical_section);
    int result = generic_queue_enqueue(queue->_generic_queue, data);
    LeaveCriticalSection(&queue->critical_section);

    return result;
}

int
generic_queue_s_dequeue(generic_queue_s queue, void** data)
{

    if (!queue || !data)
    {
        return 1;
    }

    EnterCriticalSection(&queue->critical_section);
    int result = generic_queue_dequeue(queue->_generic_queue, data);
    LeaveCriticalSection(&queue->critical_section);

    return result;
}
