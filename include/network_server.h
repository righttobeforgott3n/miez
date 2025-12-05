#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "message_broker.h"
#include <stddef.h>

typedef struct network_server_t* network_server;

struct network_server_configuration_t
{
    const char* _host;
    int _port;
    const char* _cert_file;
    const char* _key_file;
    const char* _api_key;
    struct message_broker_t* _broker;
    size_t _max_clients;
};

int
network_server_new(struct network_server_configuration_t* config,
                   struct network_server_t** out_self);

int
network_server_start(struct network_server_t* self);

int
network_server_stop(struct network_server_t* self);

int
network_server_free(struct network_server_t* self);

int
network_server_get_port(struct network_server_t* self, int* out_port);

#endif

// @todo change the name module (?)