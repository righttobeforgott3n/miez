#include "pseudo_generic_queue.h"
#include "pseudo_generic_queue_s.h"
#include <pthread.h>
#include <stdlib.h>

struct pseudo_generic_queue_s_t
{
    pseudo_generic_queue queue;
    pthread_mutex_t mutex;
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

    if (pthread_mutex_init(&queue_s->mutex, NULL) != 0)
    {
        pseudo_generic_queue_free(queue_s->queue);
        free(queue_s);
        return NULL;
    }

    return queue_s;
}

void
pseudo_generic_queue_s_free(pseudo_generic_queue_s queue)
{

    if (!queue)
    {
        return;
    }

    pthread_mutex_destroy(&queue->mutex);
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

    pthread_mutex_lock(&queue->mutex);
    size_t size = pseudo_generic_queue_size(queue->queue);
    pthread_mutex_unlock(&queue->mutex);

    return size;
}

int
pseudo_generic_queue_s_enqueue(pseudo_generic_queue_s queue, void* data)
{

    if (!queue)
    {
        return 1;
    }

    pthread_mutex_lock(&queue->mutex);
    int result = pseudo_generic_queue_enqueue(queue->queue, data);
    pthread_mutex_unlock(&queue->mutex);

    return result;
}

int
pseudo_generic_queue_s_dequeue(pseudo_generic_queue_s queue, void** data)
{
    if (!queue || !data)
    {
        return 1;
    }

    pthread_mutex_lock(&queue->mutex);
    int result = pseudo_generic_queue_dequeue(queue->queue, data);
    pthread_mutex_unlock(&queue->mutex);

    return result;
}
