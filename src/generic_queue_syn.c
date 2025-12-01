#include "generic_queue_syn.h"
#include "generic_queue.h"
#include <pthread.h>
#include <stdlib.h>

struct generic_queue_syn_t
{
    generic_queue _queue;
    pthread_mutex_t _mutex;
};

int
generic_queue_syn_new(generic_queue_syn* out_self)
{
    if (out_self == NULL)
    {
        return -1;
    }

    generic_queue_syn self = malloc(sizeof(struct generic_queue_syn_t));
    if (self == NULL)
    {
        return -1;
    }

    if (generic_queue_new(&self->_queue) != 0)
    {
        free(self);
        return -1;
    }

    if (pthread_mutex_init(&self->_mutex, NULL) != 0)
    {
        generic_queue_free(self->_queue);
        free(self);
        return -1;
    }

    *out_self = self;
    return 0;
}

int
generic_queue_syn_free(generic_queue_syn self)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_free(self->_queue);
    pthread_mutex_unlock(&self->_mutex);

    pthread_mutex_destroy(&self->_mutex);
    free(self);

    return result;
}

int
generic_queue_syn_set_free_function(generic_queue_syn self,
                                    void (*free_function)(void*))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_set_free_function(self->_queue, free_function);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_get_free_function(generic_queue_syn self,
                                    void (**out_free_function)(void*))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result =
        generic_queue_get_free_function(self->_queue, out_free_function);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_set_copy_function(generic_queue_syn self,
                                    int (*copy_function)(void*, void**))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_set_copy_function(self->_queue, copy_function);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_get_copy_function(generic_queue_syn self,
                                    int (**out_copy_function)(void*, void**))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result =
        generic_queue_get_copy_function(self->_queue, out_copy_function);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_size(generic_queue_syn self, size_t* out_size)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_size(self->_queue, out_size);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_is_empty(generic_queue_syn self)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_is_empty(self->_queue);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_enqueue(generic_queue_syn self, void* data)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_enqueue(self->_queue, data);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_dequeue(generic_queue_syn self, void** out_data)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_dequeue(self->_queue, out_data);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_peek(generic_queue_syn self, void** out_data)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_peek(self->_queue, out_data);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_peek_rear(generic_queue_syn self, void** out_data)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_peek_rear(self->_queue, out_data);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_clear(generic_queue_syn self)
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_clear(self->_queue);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_contains(generic_queue_syn self, void* target,
                           int (*compare)(void*, void*))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_contains(self->_queue, target, compare);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}

int
generic_queue_syn_for_each(generic_queue_syn self, void (*apply)(void*))
{
    if (self == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&self->_mutex);
    int result = generic_queue_for_each(self->_queue, apply);
    pthread_mutex_unlock(&self->_mutex);

    return result;
}
