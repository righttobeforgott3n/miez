#include "message_broker.h"
#include "generic_hash_table.h"
#include "generic_linked_list.h"
#include "generic_queue_syn.h"
#include "thread_pool.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct message_broker_t
{
    struct thread_pool_t* _publisher_pool;
    generic_hash_table _channels;
    pthread_mutex_t _channels_mutex;
    atomic_uint_fast64_t _next_subscriber_id;
    atomic_uint_fast64_t _next_message_id;
};

struct message_t
{
    uint64_t _id;
    char* _channel_name;
    char* _content;
};

struct subscriber_proxy_t
{
    uint64_t _id;
    generic_queue_syn _inbox;
    pthread_mutex_t _inbox_mutex;
    pthread_cond_t _inbox_cond;
    int _active;
};

struct subscription_t
{
    uint64_t _id;
    char* _channel_name;
    struct message_broker_t* _broker;
    struct subscriber_proxy_t* _proxy;
    int _active;
};

struct channel_t
{
    char* _channel_name;
    generic_linked_list _subscriber_proxies;
    pthread_mutex_t _mutex;
};

static int
_message_new(uint64_t id, const char* channel_name, const char* content,
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

    struct message_t* self = malloc(sizeof(struct message_t));
    if (!self)
    {
        return -1;
    }

    self->_id = id;

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
        free(self->_channel_name);
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

int
message_get_id(struct message_t* self, uint64_t* out_id)
{

    if (!self)
    {
        return 1;
    }

    if (!out_id)
    {
        return 1;
    }

    *out_id = self->_id;

    return 0;
}

int
message_get_channel(struct message_t* self, const char** out_channel)
{

    if (!self)
    {
        return 1;
    }

    if (!out_channel)
    {
        return 1;
    }

    *out_channel = self->_channel_name;

    return 0;
}

int
message_get_content(struct message_t* self, const char** out_content)
{

    if (!self)
    {
        return 1;
    }

    if (!out_content)
    {
        return 1;
    }

    *out_content = self->_content;

    return 0;
}

static void
_message_free_wrapper(void* data)
{

    if (!data)
    {
        return;
    }

    message_free((struct message_t*) data);
}

static int
_subscriber_proxy_new(uint64_t id, struct subscriber_proxy_t** out_self)
{

    if (!out_self)
    {
        return 1;
    }

    struct subscriber_proxy_t* self = malloc(sizeof(struct subscriber_proxy_t));
    if (!self)
    {
        return -1;
    }

    self->_id = id;
    self->_active = 1;

    int exit_code = generic_queue_syn_new(&self->_inbox);
    if (exit_code)
    {
        free(self);
        return exit_code;
    }

    generic_queue_syn_set_free_function(self->_inbox, _message_free_wrapper);

    exit_code = pthread_mutex_init(&self->_inbox_mutex, NULL);
    if (exit_code)
    {

        generic_queue_syn_free(self->_inbox);
        free(self);

        return exit_code;
    }

    exit_code = pthread_cond_init(&self->_inbox_cond, NULL);
    if (exit_code)
    {

        pthread_mutex_destroy(&self->_inbox_mutex);
        generic_queue_syn_free(self->_inbox);
        free(self);

        return exit_code;
    }

    *out_self = self;

    return 0;
}

static void
_subscriber_proxy_free(struct subscriber_proxy_t* self)
{

    if (!self)
    {
        return;
    }

    self->_active = 0;

    pthread_mutex_lock(&self->_inbox_mutex);
    pthread_cond_broadcast(&self->_inbox_cond);
    pthread_mutex_unlock(&self->_inbox_mutex);

    generic_queue_syn_free(self->_inbox);
    pthread_mutex_destroy(&self->_inbox_mutex);
    pthread_cond_destroy(&self->_inbox_cond);
    free(self);
}

static void
_subscriber_proxy_free_wrapper(void* data)
{

    if (!data)
    {
        return;
    }

    _subscriber_proxy_free((struct subscriber_proxy_t*) data);
}

static int
_subscriber_proxy_copy(void* src, void** dst)
{

    if (!src)
    {
        return 1;
    }

    if (!dst)
    {
        return 1;
    }

    *dst = src;

    return 0;
}

static int
_subscriber_proxy_enqueue(struct subscriber_proxy_t* self,
                          struct message_t* msg)
{

    if (!self)
    {
        return 1;
    }

    if (!msg)
    {
        return 1;
    }

    if (!self->_active)
    {
        return 1;
    }

    int exit_code = generic_queue_syn_enqueue(self->_inbox, msg);
    if (exit_code)
    {
        return exit_code;
    }

    pthread_mutex_lock(&self->_inbox_mutex);
    pthread_cond_signal(&self->_inbox_cond);
    pthread_mutex_unlock(&self->_inbox_mutex);

    return 0;
}

static int
_channel_new(const char* name, struct channel_t** out_self)
{

    if (!name)
    {
        return 1;
    }

    if (!out_self)
    {
        return 1;
    }

    struct channel_t* self = malloc(sizeof(struct channel_t));
    if (!self)
    {
        return -1;
    }

    size_t name_len = strlen(name);
    self->_channel_name = malloc(name_len + 1);
    if (!self->_channel_name)
    {
        free(self);
        return -1;
    }
    memcpy(self->_channel_name, name, name_len + 1);

    int exit_code = generic_linked_list_new(&self->_subscriber_proxies);
    if (exit_code)
    {

        free(self->_channel_name);
        free(self);

        return exit_code;
    }

    generic_linked_list_set_free_function(self->_subscriber_proxies,
                                          _subscriber_proxy_free_wrapper);
    generic_linked_list_set_copy_function(self->_subscriber_proxies,
                                          _subscriber_proxy_copy);

    exit_code = pthread_mutex_init(&self->_mutex, NULL);
    if (exit_code)
    {

        generic_linked_list_free(self->_subscriber_proxies);
        free(self->_channel_name);
        free(self);

        return exit_code;
    }

    *out_self = self;

    return 0;
}

static void
_channel_free(struct channel_t* self)
{

    if (!self)
    {
        return;
    }

    pthread_mutex_lock(&self->_mutex);

    generic_linked_list_free(self->_subscriber_proxies);
    free(self->_channel_name);

    pthread_mutex_unlock(&self->_mutex);
    pthread_mutex_destroy(&self->_mutex);

    free(self);
}

static void
_channel_free_wrapper(void* data)
{

    if (!data)
    {
        return;
    }

    _channel_free((struct channel_t*) data);
}

static int
_channel_copy(void* src, void** dst)
{

    if (!src)
    {
        return 1;
    }

    if (!dst)
    {
        return 1;
    }

    *dst = src;

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

    if (!src)
    {
        return 1;
    }

    if (!dst)
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

struct _publisher_task_arg_t
{
    uint64_t _message_id;
    char* _channel_name;
    char* _content;
    generic_hash_table _channels;
    pthread_mutex_t* _channels_mutex;
};

static void
_publisher_task_arg_free(struct _publisher_task_arg_t* arg)
{

    if (!arg)
    {
        return;
    }

    free(arg->_channel_name);
    free(arg->_content);
    free(arg);
}

// @todo refactor this function to avoid _channels_mutex: generic_hash_table
// must expose something like *_get_and_create as atomic function, and then
// working on the channel specific mutex
static void*
_publisher_task(void* arg)
{

    if (!arg)
    {
        return NULL;
    }

    struct _publisher_task_arg_t* task_arg =
        (struct _publisher_task_arg_t*) arg;

    pthread_mutex_lock(task_arg->_channels_mutex);

    struct channel_t* channel = NULL;
    int exit_code = generic_hash_table_get(
        task_arg->_channels, task_arg->_channel_name, (void**) &channel);

    if (exit_code || !channel)
    {

        exit_code = _channel_new(task_arg->_channel_name, &channel);
        if (exit_code)
        {

            pthread_mutex_unlock(task_arg->_channels_mutex);
            fprintf(stderr, "[message_broker] failed to create channel: %s\n",
                    task_arg->_channel_name);

            _publisher_task_arg_free(task_arg);

            return NULL;
        }

        exit_code = generic_hash_table_insert(task_arg->_channels,
                                              task_arg->_channel_name, channel);
        if (exit_code)
        {

            pthread_mutex_unlock(task_arg->_channels_mutex);

            _channel_free(channel);

            // @todo refactor the entire module the way messages are logges with
            // a
            // consisten way.
            printf("[message_broker] failed to insert channel: %s\n",
                   task_arg->_channel_name);

            _publisher_task_arg_free(task_arg);

            return NULL;
        }
    }

    pthread_mutex_unlock(task_arg->_channels_mutex);

    pthread_mutex_lock(&channel->_mutex);

    size_t subscriber_count = 0;
    generic_linked_list_size(channel->_subscriber_proxies, &subscriber_count);
    if (subscriber_count)
    {

        generic_linked_list_iterator iter = NULL;
        exit_code = generic_linked_list_iterator_begin(
            channel->_subscriber_proxies, &iter);
        if (exit_code == 0)
        {

            while (generic_linked_list_iterator_is_valid(iter) == 0)
            {

                struct subscriber_proxy_t* proxy = NULL;
                exit_code =
                    generic_linked_list_iterator_get(iter, (void**) &proxy);
                if (exit_code == 0 && proxy && proxy->_active)
                {

                    struct message_t* msg_copy = NULL;
                    exit_code = _message_new(task_arg->_message_id,
                                             task_arg->_channel_name,
                                             task_arg->_content, &msg_copy);
                    if (exit_code == 0 && msg_copy)
                    {

                        exit_code = _subscriber_proxy_enqueue(proxy, msg_copy);
                        if (exit_code)
                        {
                            message_free(msg_copy);
                        }
                    }
                }

                generic_linked_list_iterator_next(iter);
            }

            generic_linked_list_iterator_free(iter);
        }
    }

    pthread_mutex_unlock(&channel->_mutex);

    // @todo refactor the entire module the way messages are logges with a
    // consisten way.
    printf("[message_broker] published (id: %lu) channel: %s, content: %s, "
           "subscribers: %zu\n",
           (unsigned long) task_arg->_message_id, task_arg->_channel_name,
           task_arg->_content, subscriber_count);

    _publisher_task_arg_free(task_arg);

    return NULL;
}

int
message_broker_new(struct message_broker_configuration_t* config,
                   struct message_broker_t** out_self)
{

    if (!out_self)
    {
        return 1;
    }

    if (!config)
    {
        return 1;
    }

    if (!config->_n_threads)
    {
        return 1;
    }

    if (!config->_channels_capacity)
    {
        return 1;
    }

    struct message_broker_t* self = malloc(sizeof(struct message_broker_t));
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
                                       _channel_free_wrapper, _channel_copy,
                                       _string_free, _string_copy,
                                       _string_compare, &self->_channels);
    if (exit_code)
    {

        thread_pool_free(self->_publisher_pool);
        free(self);

        return exit_code;
    }

    exit_code = pthread_mutex_init(&self->_channels_mutex, NULL);
    if (exit_code)
    {

        generic_hash_table_free(self->_channels);
        thread_pool_free(self->_publisher_pool);
        free(self);

        return exit_code;
    }

    atomic_init(&self->_next_subscriber_id, 1);
    atomic_init(&self->_next_message_id, 1);

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

    thread_pool_free(self->_publisher_pool);
    generic_hash_table_free(self->_channels);
    pthread_mutex_destroy(&self->_channels_mutex);
    free(self);

    return 0;
}

int
message_broker_publish(struct message_broker_t* self, const char* channel,
                       const char* content)
{

    if (!self)
    {
        return 1;
    }

    if (!channel)
    {
        return 1;
    }

    if (!content)
    {
        return 1;
    }

    struct _publisher_task_arg_t* task_arg =
        malloc(sizeof(struct _publisher_task_arg_t));
    if (!task_arg)
    {
        return -1;
    }

    task_arg->_message_id = atomic_fetch_add(&self->_next_message_id, 1);

    size_t channel_len = strlen(channel);
    task_arg->_channel_name = malloc(channel_len + 1);
    if (!task_arg->_channel_name)
    {
        free(task_arg);
        return -1;
    }
    memcpy(task_arg->_channel_name, channel, channel_len + 1);

    size_t content_len = strlen(content);
    task_arg->_content = malloc(content_len + 1);
    if (!task_arg->_content)
    {
        free(task_arg->_channel_name);
        free(task_arg);
        return -1;
    }
    memcpy(task_arg->_content, content, content_len + 1);

    task_arg->_channels = self->_channels;
    task_arg->_channels_mutex = &self->_channels_mutex;

    int exit_code =
        thread_pool_submit(self->_publisher_pool, _publisher_task, task_arg);
    if (exit_code)
    {
        _publisher_task_arg_free(task_arg);
        return exit_code;
    }

    return 0;
}

// @todo refactor this function to avoid _channels_mutex
int
message_broker_subscribe(struct message_broker_t* self, const char* channel,
                         struct subscription_t** out_subscription)
{

    if (!self)
    {
        return 1;
    }

    if (!channel)
    {
        return 1;
    }

    if (!out_subscription)
    {
        return 1;
    }

    uint64_t subscriber_id = atomic_fetch_add(&self->_next_subscriber_id, 1);

    pthread_mutex_lock(&self->_channels_mutex);

    struct channel_t* ch = NULL;
    int exit_code =
        generic_hash_table_get(self->_channels, (void*) channel, (void**) &ch);

    // @todo unify this snippet under the same utility, it is used from the
    // publisher worker too.
    if (exit_code || !ch)
    {

        exit_code = _channel_new(channel, &ch);
        if (exit_code)
        {
            pthread_mutex_unlock(&self->_channels_mutex);
            return exit_code;
        }

        exit_code =
            generic_hash_table_insert(self->_channels, (void*) channel, ch);
        if (exit_code)
        {

            _channel_free(ch);
            pthread_mutex_unlock(&self->_channels_mutex);

            return exit_code;
        }
    }

    pthread_mutex_unlock(&self->_channels_mutex);

    struct subscriber_proxy_t* proxy = NULL;
    exit_code = _subscriber_proxy_new(subscriber_id, &proxy);
    if (exit_code)
    {
        return exit_code;
    }

    pthread_mutex_lock(&ch->_mutex);
    exit_code = generic_linked_list_insert_last(ch->_subscriber_proxies, proxy);
    pthread_mutex_unlock(&ch->_mutex);

    if (exit_code)
    {
        _subscriber_proxy_free(proxy);
        return exit_code;
    }

    struct subscription_t* subscription = malloc(sizeof(struct subscription_t));
    if (!subscription)
    {
        return -1;
    }

    subscription->_id = subscriber_id;
    subscription->_broker = self;
    subscription->_proxy = proxy;
    subscription->_active = 1;

    size_t channel_len = strlen(channel);
    subscription->_channel_name = malloc(channel_len + 1);
    if (!subscription->_channel_name)
    {
        free(subscription);
        return -1;
    }
    memcpy(subscription->_channel_name, channel, channel_len + 1);

    *out_subscription = subscription;

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

int
subscription_receive(struct subscription_t* self, struct message_t** out_msg)
{

    if (!self)
    {
        return 1;
    }

    if (!out_msg)
    {
        return 1;
    }

    if (!self->_active)
    {
        return 1;
    }

    if (!self->_proxy)
    {
        return 1;
    }

    if (!self->_proxy->_active)
    {
        return 1;
    }

    struct subscriber_proxy_t* proxy = self->_proxy;

    pthread_mutex_lock(&proxy->_inbox_mutex);

    while (generic_queue_syn_is_empty(proxy->_inbox) == 1 && proxy->_active)
    {
        pthread_cond_wait(&proxy->_inbox_cond, &proxy->_inbox_mutex);
    }

    if (!proxy->_active)
    {

        pthread_mutex_unlock(&proxy->_inbox_mutex);
        *out_msg = NULL;

        return 1;
    }

    pthread_mutex_unlock(&proxy->_inbox_mutex);

    struct message_t* msg = NULL;
    int exit_code = generic_queue_syn_dequeue(proxy->_inbox, (void**) &msg);
    if (exit_code)
    {
        *out_msg = NULL;
        return exit_code;
    }

    *out_msg = msg;

    return 0;
}

int
subscription_try_receive(struct subscription_t* self,
                         struct message_t** out_msg)
{

    if (!self)
    {
        return 1;
    }

    if (!out_msg)
    {
        return 1;
    }

    if (!self->_active)
    {
        return 1;
    }

    if (!self->_proxy)
    {
        return 1;
    }

    if (!self->_proxy->_active)
    {
        return 1;
    }

    struct subscriber_proxy_t* proxy = self->_proxy;

    if (generic_queue_syn_is_empty(proxy->_inbox) == 1)
    {
        *out_msg = NULL;
        return 1;
    }

    struct message_t* msg = NULL;
    int exit_code = generic_queue_syn_dequeue(proxy->_inbox, (void**) &msg);
    if (exit_code)
    {
        *out_msg = NULL;
        return exit_code;
    }

    *out_msg = msg;

    return 0;
}

int
subscription_unsubscribe(struct subscription_t* self)
{

    if (!self)
    {
        return 1;
    }

    if (!self->_active)
    {
        return 1;
    }

    struct message_broker_t* broker = self->_broker;
    if (!broker)
    {
        return 1;
    }

    if (self->_proxy)
    {

        self->_proxy->_active = 0;

        pthread_mutex_lock(&self->_proxy->_inbox_mutex);
        pthread_cond_broadcast(&self->_proxy->_inbox_cond);
        pthread_mutex_unlock(&self->_proxy->_inbox_mutex);
    }

    pthread_mutex_lock(&broker->_channels_mutex);

    struct channel_t* ch = NULL;
    int exit_code = generic_hash_table_get(
        broker->_channels, (void*) self->_channel_name, (void**) &ch);
    if (exit_code == 0 && ch)
    {

        pthread_mutex_lock(&ch->_mutex);

        generic_linked_list_iterator iter = NULL;
        exit_code =
            generic_linked_list_iterator_begin(ch->_subscriber_proxies, &iter);
        if (exit_code == 0)
        {

            while (generic_linked_list_iterator_is_valid(iter) == 0)
            {

                struct subscriber_proxy_t* proxy = NULL;
                exit_code =
                    generic_linked_list_iterator_get(iter, (void**) &proxy);
                if (exit_code == 0 && proxy && proxy->_id == self->_id)
                {

                    generic_linked_list_iterator_remove(iter, NULL);
                    break;
                }

                generic_linked_list_iterator_next(iter);
            }

            generic_linked_list_iterator_free(iter);
        }

        pthread_mutex_unlock(&ch->_mutex);
    }

    pthread_mutex_unlock(&broker->_channels_mutex);

    self->_active = 0;
    self->_proxy = NULL;

    return 0;
}

int
subscription_free(struct subscription_t* self)
{

    if (!self)
    {
        return 1;
    }

    if (self->_active)
    {
        subscription_unsubscribe(self);
    }

    free(self->_channel_name);
    free(self);

    return 0;
}

int
subscription_get_id(struct subscription_t* self, uint64_t* out_id)
{

    if (!self)
    {
        return 1;
    }

    if (!out_id)
    {
        return 1;
    }

    *out_id = self->_id;

    return 0;
}

int
subscription_get_channel(struct subscription_t* self, const char** out_channel)
{

    if (!self)
    {
        return 1;
    }

    if (!out_channel)
    {
        return 1;
    }

    *out_channel = self->_channel_name;

    return 0;
}

int
subscription_get_pending_count(struct subscription_t* self, size_t* out_count)
{

    if (!self)
    {
        return 1;
    }

    if (!out_count)
    {
        return 1;
    }

    if (!self->_proxy)
    {
        *out_count = 0;
        return 0;
    }

    return generic_queue_syn_size(self->_proxy->_inbox, out_count);
}

// @todo publisher is anonymous in the current release, setting up a
// registration phase could be useful in future for many reasons.
// @todo the channel persists with the message broker lifetime, to avoid memory
// consumption a release channel api should be implemented.
// @todo I suppose an improvement could be made on the subscriber side: as for
// the publisher, instead of waiting for the caller thread to complete the sub
// operation, I task to an internal thread pool could be submitted.

// @todo I would like to avoid the usage of _channels_mutex within the struct
// message_broker_t