#ifndef MESSAGE_BROKER_H
#define MESSAGE_BROKER_H

#include <stddef.h>

typedef struct message_broker_t* message_broker;
typedef struct message_broker_configuration_t* message_broker_configuration;

struct message_broker_configuration_t
{
    size_t _n_threads;
};

int
message_broker_new(struct message_broker_configuration_t* config,
                   struct message_broker_t** out_self);

int
message_broker_free(struct message_broker_t* self);

int
message_broker_publish(struct message_broker_t* self, const char* channel,
                       const char* message);

int
message_broker_wait(struct message_broker_t* self);

#endif  // MESSAGE_BROKER_H
