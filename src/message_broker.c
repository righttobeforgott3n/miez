#include "message_broker.h"
#include "generic_hash_table.h"
#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message_broker_t
{
    struct thread_pool_t* _thread_pool;
    generic_hash_table _channels;
};

struct message_t
{
    char* _channel;
    char* _content;
};

int
message_new(const char* channel, const char* content, message* out_self)
{

    if (!channel)
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

    struct message_t* self = malloc(sizeof(struct message_t));
    if (!self)
    {
        return -1;
    }

    size_t channel_len = strlen(channel);
    self->_channel = malloc(channel_len + 1);
    if (!self->_channel)
    {
        free(self);
        return -1;
    }
    memcpy(self->_channel, channel, channel_len + 1);

    size_t content_len = strlen(content);
    self->_content = malloc(content_len + 1);
    if (!self->_content)
    {

        free(self->_channel);
        free(self);

        return -1;
    }
    memcpy(self->_content, content, content_len + 1);

    *out_self = self;

    return 0;
}

int
message_free(message self)
{

    if (!self)
    {
        return 1;
    }

    free(self->_channel);
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

// @todo just a test: need to be modified to implement the rest of the business.
static void*
_publisher_task(void* arg)
{

    if (!arg)
    {
        return NULL;
    }

    message msg = (message) arg;

    printf("[message_broker] channel: %s, message: %s\n", msg->_channel,
           msg->_content);

    message_free(msg);

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

    int exit_code = thread_pool_new(config->_n_threads, &self->_thread_pool);
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

        thread_pool_free(self->_thread_pool);
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

    thread_pool_free(self->_thread_pool);      // @todo check exit code and log
    generic_hash_table_free(self->_channels);  // @todo check exit code and log
    free(self);

    return 0;
}

int
message_broker_publish(struct message_broker_t* self, message m)
{
    if (!self)
    {
        return 1;
    }

    if (!m)
    {
        return 1;
    }

    int exit_code = thread_pool_submit(self->_thread_pool, _publisher_task, m);
    if (exit_code)
    {
        message_free(m);
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