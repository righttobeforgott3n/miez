#include "message_broker.h"
#include "generic_hash_table.h"
#include "generic_linked_list.h"
#include "generic_queue_syn.h"
#include "thread_pool.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message_broker_t
{

    struct thread_pool_t* _publisher_pool;
    generic_hash_table _channels;
};

struct message_t
{

    char* _channel_name;
    char* _content;
};

struct subscriber_proxy_t
{
    // @todo queue syn of messages.
};

struct channel_t
{

    char* _channel_name;
    generic_linked_list _subscriber_proxies;
    pthread_mutex_t* _mutex;
};

int
message_new(char* channel_name, const char* content,
            struct message_t** out_self)
{

    if (!channel_name)
    {
        return 1;
    }

    if (!content)
    {
        return 1;
    }

    if (!out_self)
    {
        return 1;
    }

    struct message_t* self =
        (struct message_t*) malloc(sizeof(struct message_t));
    if (!self)
    {
        return -1;
    }

    size_t channel_len = strlen(channel_name);
    self->_channel_name = malloc(channel_len + 1);
    if (!self->_channel_name)
    {
        free(self);
        return -1;
    }
    memcpy(self->_channel_name, channel_name, channel_len + 1);

    size_t content_len = strlen(content);
    self->_content = malloc(content_len + 1);
    if (!self->_content)
    {

        free(self);
        return -1;
    }
    memcpy(self->_content, content, content_len + 1);

    *out_self = self;

    return 0;
}

int
message_free(struct message_t* self)
{

    if (!self)
    {
        return 1;
    }

    free(self->_channel_name);
    free(self->_content);
    free(self);

    return 0;
}

static size_t
_string_hash(void* key)
{

    size_t hash = 5381;
    unsigned char* p = (unsigned char*) key;
    while (*p)
    {
        hash = ((hash << 5) + hash) + *p++;
    }

    return hash;
}

static void
_string_free(void* data)
{
    free(data);
}

static int
_string_copy(void* src, void** dst)
{

    if (!src || !dst)
    {
        return 1;
    }

    size_t len = strlen((char*) src);
    char* copy = malloc(len + 1);
    if (!copy)
    {
        return -1;
    }

    memcpy(copy, src, len + 1);
    *dst = copy;

    return 0;
}

static int
_string_compare(void* a, void* b)
{
    return strcmp((char*) a, (char*) b);
}

// @todo placeholder for channel value - will be replaced with proper structure
static void
_channel_value_free(void* data)
{
    free(data);
}

static int
_channel_value_copy(void* src, void** dst)
{

    if (!src || !dst)
    {
        return 1;
    }

    // @todo placeholder - just copy pointer for now
    *dst = src;
    return 0;
}

struct _publisher_task_arg_t
{

    char* _channel_name;
    char* _message;
    generic_hash_table _channel_table;
};

// @todo just a test: need to be modified to implement the rest of the business.
static void*
_publisher_task(void* arg)
{

    if (!arg)
    {
        return NULL;
    }

    struct _publisher_task_arg_t* publisher_arg =
        (struct _publisher_task_arg_t*) arg;

    // @todo here: channel_name and message could be copied in way to avoid
    // the free call after.
    printf("[message_broker] channel: %s, message: %s\n",
           publisher_arg->_channel_name, publisher_arg->_message);

    free(publisher_arg);  // @todo if the publisher arg structure starts to grow
                          // in complexity then implement a free helper.

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

    if (!config->_channels_capacity)  // @todo upper value check.
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

    int exit_code = thread_pool_new(config->_n_threads, &self->_publisher_pool);
    if (exit_code)
    {
        free(self);
        return exit_code;
    }

    exit_code = generic_hash_table_new(config->_channels_capacity, _string_hash,
                                       _channel_value_free, _channel_value_copy,
                                       _string_free, _string_copy,
                                       _string_compare, &self->_channels);
    if (exit_code)
    {

        thread_pool_free(self->_publisher_pool);
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

    thread_pool_free(self->_publisher_pool);   // @todo check exit code and log
    generic_hash_table_free(self->_channels);  // @todo check exit code and log
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

    struct _publisher_task_arg_t* publisher_arg =
        (struct _publisher_task_arg_t*) malloc(
            sizeof(struct _publisher_task_arg_t));
    if (!publisher_arg)
    {
        return -1;
    }

    publisher_arg->_channel_name = (char*) channel;
    publisher_arg->_message = (char*) message;
    publisher_arg->_channel_table = self->_channels;

    // @todo it would be useful here to have a thread pool which frees the
    // passed argument in way to speed up the caller thread in case of errors.
    int exit_code = thread_pool_submit(self->_publisher_pool, _publisher_task,
                                       (void*) publisher_arg);
    if (exit_code)
    {

        free(publisher_arg->_channel_name);
        free(publisher_arg->_message);
        free(publisher_arg);

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

    return thread_pool_wait(self->_publisher_pool);
}

// @todo the entire module is in progress...