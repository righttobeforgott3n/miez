#include "generic_queue.h"
#include "generic_queue_s.h"
#include <pthread.h>
#include <stdlib.h>

struct generic_queue_s_t
{
    generic_queue _generic_queue;
    pthread_mutex_t mutex;
};

int
generic_queue_s_new(generic_queue_s* out_self)
{

    if (!out_self)
    {
        return 1;
    }

    *out_self = (generic_queue_s) malloc(sizeof(struct generic_queue_s_t));
    if (!*out_self)
    {
        return -1;
    }

    int result = generic_queue_new(&((*out_self)->_generic_queue));
    if (result && !(*out_self)->_generic_queue)
    {
        free(*out_self);
        return result;
    }

    if (pthread_mutex_init(&(*out_self)->mutex, NULL) != 0)
    {

        generic_queue_free((*out_self)->_generic_queue);
        free(out_self);

        return -1;
    }

    return 0;
}

int
generic_queue_s_free(generic_queue_s self)
{

    if (!self)
    {
        return 1;
    }

    pthread_mutex_destroy(&self->mutex);
    generic_queue_free(self->_generic_queue);
    free(self);

    return 0;
}

int
generic_queue_s_size(generic_queue_s self, size_t* out_size)
{

    if (!self)
    {
        return 1;
    }

    pthread_mutex_lock(&self->mutex);
    int result = generic_queue_size(self->_generic_queue, out_size);
    // @todo add check on exit code, set out_size on NULL -> unlock -> return
    // exit_code
    pthread_mutex_unlock(&self->mutex);

    return result;
}

int
generic_queue_s_enqueue(generic_queue_s self, void* data)
{

    if (!self)
    {
        return 1;
    }

    pthread_mutex_lock(&self->mutex);
    int result = generic_queue_enqueue(self->_generic_queue, data);
    // @todo add check on exit code -> unlock -> return
    // exit_code
    pthread_mutex_unlock(&self->mutex);

    return result;
}

int
generic_queue_s_dequeue(generic_queue_s self, void** out_data)
{
    if (!self || !out_data)
    {
        return 1;
    }

    pthread_mutex_lock(&self->mutex);
    int result = generic_queue_dequeue(self->_generic_queue, out_data);
    // @todo add check on exit code -> unlock -> set out_data on NULL -> return
    // exit_code
    pthread_mutex_unlock(&self->mutex);

    return result;
}
