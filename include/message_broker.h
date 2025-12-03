#ifndef MESSAGE_BROKER_H
#define MESSAGE_BROKER_H

#include <stddef.h>

typedef struct message_broker_t* message_broker;
typedef struct message_broker_configuration_t* message_broker_configuration;

typedef struct message_t* message;

struct message_broker_configuration_t
{
    size_t _n_threads;
    size_t _channels_capacity;
};

int
message_new(const char* channel, const char* content, message* out_self);

int
message_free(message self);

int
message_broker_new(struct message_broker_configuration_t* config,
                   struct message_broker_t** out_self);

int
message_broker_free(struct message_broker_t* self);

int
message_broker_publish(struct message_broker_t* self, message m);

int
message_broker_wait(struct message_broker_t* self);

#endif  // MESSAGE_BROKER_H
