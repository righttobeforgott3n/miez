#include "message_broker.h"
#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message_broker_t
{
    struct thread_pool_t* _thread_pool;
    // @todo other attributes are needed.
};

// @todo just a test: need to be modified to implement the rest of the business.
struct publish_task_arg_t
{
    char* _channel;
    char* _message;
};

// @todo just a test: need to be modified to implement the rest of the business.
static void
_publish_task_arg_free(struct publish_task_arg_t* arg)
{

    if (!arg)
    {
        return;
    }

    free(arg->_channel);
    free(arg->_message);
    free(arg);
}

// @todo just a test: need to be modified to implement the rest of the business.
static void*
_publish_task(void* arg)
{

    if (!arg)
    {
        return NULL;
    }

    struct publish_task_arg_t* task_arg = (struct publish_task_arg_t*) arg;

    printf("[message_broker] channel: %s, message: %s\n", task_arg->_channel,
           task_arg->_message);

    _publish_task_arg_free(task_arg);

    return NULL;
}

int
message_broker_new(struct message_broker_configuration_t* config,
                   struct message_broker_t** out_self)
{

    if (!out_self)
    {
        // @todo log
        return 1;
    }

    if (!config)
    {
        // @todo log
        return 1;
    }

    if (!config->_n_threads)  // @todo upper value check.
    {
        // @todo log
        return 1;
    }

    struct message_broker_t* self =
        (struct message_broker_t*) malloc(sizeof(struct message_broker_t));
    if (!self)
    {
        return -1;
    }

    int exit_code = thread_pool_new(config->_n_threads, &self->_thread_pool);
    if (exit_code)
    {
        free(self);
        return exit_code;
    }

    *out_self = self;

    return 0;
}

int
message_broker_free(struct message_broker_t* self)
{

    if (!self)
    {
        return 1;
    }

    thread_pool_free(self->_thread_pool);  // @todo check exit code and log
    free(self);

    return 0;
}

int
message_broker_publish(struct message_broker_t* self, const char* channel,
                       const char* message)
{

    if (!self)
    {
        return 1;
    }

    if (!channel)
    {
        return 1;
    }

    if (!message)
    {
        return 1;
    }

    struct publish_task_arg_t* task_arg =
        malloc(sizeof(struct publish_task_arg_t));
    if (!task_arg)
    {
        return -1;
    }

    size_t channel_len = strlen(channel);
    task_arg->_channel = malloc(channel_len + 1);
    if (!task_arg->_channel)
    {
        free(task_arg);
        return -1;
    }
    memcpy(task_arg->_channel, channel, channel_len + 1);

    size_t message_len = strlen(message);
    task_arg->_message = malloc(message_len + 1);
    if (!task_arg->_message)
    {
        free(task_arg->_channel);
        free(task_arg);
        return -1;
    }
    memcpy(task_arg->_message, message, message_len + 1);

    int exit_code =
        thread_pool_submit(self->_thread_pool, _publish_task, task_arg);
    if (exit_code)
    {
        _publish_task_arg_free(task_arg);
        return exit_code;
    }

    return 0;
}

int
message_broker_wait(struct message_broker_t* self)
{

    if (!self)
    {
        return 1;
    }

    return thread_pool_wait(self->_thread_pool);
}

// @todo the entire module is in progress...