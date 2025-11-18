#ifndef SUBSCRIBER_PROXY_H
#define SUBSCRIBER_PROXY_H

#include <stddef.h>

typedef struct subscriber_proxy_t* subscriber_proxy;

int
subscriber_proxy_new(size_t id, subscriber_proxy* self_out);

int
subscriber_proxy_free(subscriber_proxy self);

#endif  // SUBSCRIBER_PROXY_H