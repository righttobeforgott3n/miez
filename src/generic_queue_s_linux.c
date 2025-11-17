#include "generic_queue.h"
#include "generic_queue_s.h"
#include <pthread.h>
#include <stdlib.h>

struct generic_queue_s_t
{
    generic_queue queue;
    pthread_mutex_t mutex;
};

generic_queue_s
generic_queue_s_new()
{

    generic_queue_s queue_s = (generic_queue_s) malloc(
        sizeof(struct generic_queue_s_t));

    if (!queue_s)
    {
        return NULL;
    }

    queue_s->queue = generic_queue_new();
    if (!queue_s->queue)
    {
        free(queue_s);
        return NULL;
    }

    if (pthread_mutex_init(&queue_s->mutex, NULL) != 0)
    {
        generic_queue_free(queue_s->queue);
        free(queue_s);
        return NULL;
    }

    return queue_s;
}

void
generic_queue_s_free(generic_queue_s queue)
{

    if (!queue)
    {
        return;
    }

    pthread_mutex_destroy(&queue->mutex);
    generic_queue_free(queue->queue);
    free(queue);
}

size_t
generic_queue_s_size(generic_queue_s queue)
{
    if (!queue)
    {
        return 0;
    }

    pthread_mutex_lock(&queue->mutex);
    size_t size = generic_queue_size(queue->queue);
    pthread_mutex_unlock(&queue->mutex);

    return size;
}

int
generic_queue_s_enqueue(generic_queue_s queue, void* data)
{

    if (!queue)
    {
        return 1;
    }

    pthread_mutex_lock(&queue->mutex);
    int result = generic_queue_enqueue(queue->queue, data);
    pthread_mutex_unlock(&queue->mutex);

    return result;
}

int
generic_queue_s_dequeue(generic_queue_s queue, void** data)
{
    if (!queue || !data)
    {
        return 1;
    }

    pthread_mutex_lock(&queue->mutex);
    int result = generic_queue_dequeue(queue->queue, data);
    pthread_mutex_unlock(&queue->mutex);

    return result;
}
