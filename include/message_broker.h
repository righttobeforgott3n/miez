#ifndef MESSAGE_BROKER_H
#define MESSAGE_BROKER_H

#include <stddef.h>
#include <stdint.h>

typedef struct message_broker_t* message_broker;
typedef struct message_t* message;
typedef struct subscription_t* subscription;

struct message_broker_configuration_t
{
    size_t _n_threads;
    size_t _channels_capacity;
};

int
message_broker_new(struct message_broker_configuration_t* config,
                   struct message_broker_t** out_self);

int
message_broker_free(struct message_broker_t* self);

int
message_broker_publish(struct message_broker_t* self, const char* channel,
                       const char* content);

int
message_broker_subscribe(struct message_broker_t* self, const char* channel,
                         struct subscription_t** out_subscription);

int
message_broker_wait(struct message_broker_t* self);

int
message_get_id(struct message_t* self, uint64_t* out_id);

int
message_get_channel(struct message_t* self, const char** out_channel);

int
message_get_content(struct message_t* self, const char** out_content);

int
message_free(struct message_t* self);

int
subscription_receive(struct subscription_t* self, struct message_t** out_msg);

int
subscription_try_receive(struct subscription_t* self,
                         struct message_t** out_msg);

int
subscription_unsubscribe(struct subscription_t* self);

int
subscription_free(struct subscription_t* self);

int
subscription_get_id(struct subscription_t* self, uint64_t* out_id);

int
subscription_get_channel(struct subscription_t* self, const char** out_channel);

int
subscription_get_pending_count(struct subscription_t* self, size_t* out_count);

#endif