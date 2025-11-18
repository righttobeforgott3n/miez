#include "subscriber_proxy.h"
#include "generic_queue_s.h"
#include <stddef.h>
#include <stdlib.h>

struct subscriber_proxy_t
{

    size_t _id;

    generic_queue_s _generic_queue_s;
};

int
subscriber_proxy_new(size_t id, subscriber_proxy* self_out)
{

    if (!self_out)
    {
        return 1;
    }

    *self_out = (subscriber_proxy) malloc(sizeof(struct subscriber_proxy_t));
    if (!*self_out)
    {
        return -1;
    }

    (*self_out)->_id = id;
    int result = generic_queue_s_new(&((*self_out)->_generic_queue_s));
    if (result && !(*self_out)->_generic_queue_s)
    {

        free(*self_out);
        *self_out = NULL;

        return result;
    }

    return result;
}

int
subscriber_proxy_free(subscriber_proxy self)
{

    // @todo dequeue all the messages from the queue.
    // @todo free them.
    // @todo free the queue.
    free(self);

    return 0;
}