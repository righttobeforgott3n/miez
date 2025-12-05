#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "network_server.h"
#include "message_broker.h"
#include "thread_pool.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_CHANNEL_NAME 256
#define MAX_CONTENT_SIZE 65536
#define MAX_DETACHED_SUBSCRIPTIONS 1024
#define MAX_API_KEY_LEN 256

struct detached_subscription_t
{
    uint64_t _id;
    struct subscription_t* _subscription;
    struct detached_subscription_t* _next;
};

struct network_server_t
{
    struct message_broker_t* _broker;
    SSL_CTX* _ssl_ctx;
    int _server_fd;
    int _port;
    atomic_bool _running;
    pthread_t _accept_thread;
    struct thread_pool_t* _client_pool;
    size_t _max_clients;
    struct detached_subscription_t* _detached_subscriptions;
    pthread_mutex_t _detached_mutex;
    char* _api_key;
};

struct client_context_t
{
    struct network_server_t* _server;
    SSL* _ssl;
    int _client_fd;
    struct subscription_t* _subscription;
    pthread_t _receiver_thread;
    atomic_bool _active;
    int _detached;
    int _authenticated;
};

static struct subscription_t*
_find_and_remove_detached(struct network_server_t* server, uint64_t id)
{
    pthread_mutex_lock(&server->_detached_mutex);

    struct detached_subscription_t** pp = &server->_detached_subscriptions;
    while (*pp)
    {
        if ((*pp)->_id == id)
        {
            struct detached_subscription_t* found = *pp;
            struct subscription_t* sub = found->_subscription;
            *pp = found->_next;
            free(found);
            pthread_mutex_unlock(&server->_detached_mutex);
            return sub;
        }
        pp = &(*pp)->_next;
    }

    pthread_mutex_unlock(&server->_detached_mutex);
    return NULL;
}

static int
_add_detached(struct network_server_t* server, struct subscription_t* sub)
{
    uint64_t id;
    subscription_get_id(sub, &id);

    struct detached_subscription_t* node =
        malloc(sizeof(struct detached_subscription_t));
    if (!node)
    {
        return -1;
    }

    node->_id = id;
    node->_subscription = sub;

    pthread_mutex_lock(&server->_detached_mutex);
    node->_next = server->_detached_subscriptions;
    server->_detached_subscriptions = node;
    pthread_mutex_unlock(&server->_detached_mutex);

    return 0;
}

static void
_log_ssl_error(const char* msg)
{

    unsigned long err;
    while ((err = ERR_get_error()))
    {

        char buf[256];
        ERR_error_string_n(err, buf, sizeof(buf));
        fprintf(stderr, "[network_server] %s: %s\n", msg, buf);
    }
}

static SSL_CTX*
_create_ssl_context(const char* cert_file, const char* key_file)
{

    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx)
    {

        _log_ssl_error("Failed to create SSL context");
        return NULL;
    }

    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0)
    {

        _log_ssl_error("Failed to load certificate");
        SSL_CTX_free(ctx);

        return NULL;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0)
    {

        _log_ssl_error("Failed to load private key");
        SSL_CTX_free(ctx);

        return NULL;
    }

    if (!SSL_CTX_check_private_key(ctx))
    {

        fprintf(stderr,
                "[network_server] Private key does not match certificate\n");
        SSL_CTX_free(ctx);

        return NULL;
    }

    return ctx;
}

static int
_send_response(SSL* ssl, const char* response)
{

    size_t len = strlen(response);
    int written = SSL_write(ssl, response, (int) len);
    if (written <= 0)
    {

        _log_ssl_error("SSL_write failed");
        return -1;
    }

    return 0;
}

static void*
_subscriber_receiver_thread(void* arg)
{

    struct client_context_t* ctx = (struct client_context_t*) arg;
    struct message_t* msg = NULL;

    while (atomic_load(&ctx->_active))
    {

        int result = subscription_try_receive(ctx->_subscription, &msg);
        if (result == 0 && msg)
        {

            uint64_t id;
            const char* channel;
            const char* content;

            message_get_id(msg, &id);
            message_get_channel(msg, &channel);
            message_get_content(msg, &content);

            size_t content_len = strlen(content);
            char header[512];
            snprintf(header, sizeof(header), "MSG %lu %s %zu\n", id, channel,
                     content_len);

            if (_send_response(ctx->_ssl, header) == 0)
            {
                SSL_write(ctx->_ssl, content, (int) content_len);
                SSL_write(ctx->_ssl, "\n", 1);
            }

            message_free(msg);
            msg = NULL;
        }
        else
        {
            usleep(10000);
        }
    }

    return NULL;
}

static int
_handle_subscribe(struct client_context_t* ctx, const char* channel_name)
{

    if (ctx->_subscription)
    {

        _send_response(ctx->_ssl, "ERR Already subscribed\n");
        return -1;
    }

    int result = message_broker_subscribe(ctx->_server->_broker, channel_name,
                                          &ctx->_subscription);
    if (result != 0)
    {

        _send_response(ctx->_ssl, "ERR Failed to subscribe\n");
        return -1;
    }

    ctx->_detached = 0;
    atomic_store(&ctx->_active, 1);
    pthread_create(&ctx->_receiver_thread, NULL, _subscriber_receiver_thread,
                   ctx);

    uint64_t sub_id;
    subscription_get_id(ctx->_subscription, &sub_id);
    char response[64];
    snprintf(response, sizeof(response), "OK %lu\n", sub_id);
    _send_response(ctx->_ssl, response);

    return 0;
}

static int
_handle_attach(struct client_context_t* ctx, uint64_t subscription_id)
{
    if (ctx->_subscription)
    {
        _send_response(ctx->_ssl, "ERR Already subscribed\n");
        return -1;
    }

    struct subscription_t* sub =
        _find_and_remove_detached(ctx->_server, subscription_id);
    if (!sub)
    {
        _send_response(ctx->_ssl, "ERR Subscription not found\n");
        return -1;
    }

    ctx->_subscription = sub;
    ctx->_detached = 0;
    atomic_store(&ctx->_active, 1);
    pthread_create(&ctx->_receiver_thread, NULL, _subscriber_receiver_thread,
                   ctx);

    size_t pending = 0;
    subscription_get_pending_count(ctx->_subscription, &pending);
    char response[64];
    snprintf(response, sizeof(response), "OK %zu\n", pending);
    _send_response(ctx->_ssl, response);

    return 0;
}

static int
_handle_detach(struct client_context_t* ctx)
{
    if (!ctx->_subscription)
    {
        _send_response(ctx->_ssl, "ERR Not subscribed\n");
        return -1;
    }

    // Stop the receiver thread
    atomic_store(&ctx->_active, 0);
    pthread_join(ctx->_receiver_thread, NULL);

    // Store subscription for later reconnection
    if (_add_detached(ctx->_server, ctx->_subscription) != 0)
    {
        _send_response(ctx->_ssl, "ERR Failed to detach\n");
        return -1;
    }

    uint64_t sub_id;
    subscription_get_id(ctx->_subscription, &sub_id);

    // Mark as detached so cleanup doesn't free it
    ctx->_detached = 1;
    ctx->_subscription = NULL;

    char response[64];
    snprintf(response, sizeof(response), "OK %lu\n", sub_id);
    _send_response(ctx->_ssl, response);

    return 0;
}

static int
_handle_publish(struct client_context_t* ctx, const char* channel_name,
                size_t content_len)
{

    if (content_len > MAX_CONTENT_SIZE)
    {

        _send_response(ctx->_ssl, "ERR Content too large\n");
        return -1;
    }

    char* content = malloc(content_len + 1);
    if (!content)
    {

        _send_response(ctx->_ssl, "ERR Out of memory\n");
        return -1;
    }

    int bytes_read = SSL_read(ctx->_ssl, content, (int) content_len);
    if (bytes_read != (int) content_len)
    {

        free(content);
        _send_response(ctx->_ssl, "ERR Failed to read content\n");

        return -1;
    }
    content[content_len] = '\0';

    char newline;
    SSL_read(ctx->_ssl, &newline, 1);

    int result =
        message_broker_publish(ctx->_server->_broker, channel_name, content);
    free(content);
    if (result != 0)
    {

        _send_response(ctx->_ssl, "ERR Failed to publish\n");
        return -1;
    }

    _send_response(ctx->_ssl, "OK\n");

    return 0;
}

static int
_handle_auth(struct client_context_t* ctx, const char* api_key)
{
    if (ctx->_authenticated)
    {
        _send_response(ctx->_ssl, "ERR Already authenticated\n");
        return -1;
    }

    if (!ctx->_server->_api_key)
    {
        ctx->_authenticated = 1;
        _send_response(ctx->_ssl, "OK\n");
        return 0;
    }

    if (strcmp(api_key, ctx->_server->_api_key) == 0)
    {
        ctx->_authenticated = 1;
        _send_response(ctx->_ssl, "OK\n");
        return 0;
    }

    _send_response(ctx->_ssl, "ERR Invalid API key\n");
    return -1;
}

static void*
_client_handler(void* arg)
{
    struct client_context_t* ctx = (struct client_context_t*) arg;
    char buffer[BUFFER_SIZE];
    int running = 1;

    while (running && atomic_load(&ctx->_server->_running))
    {
        memset(buffer, 0, sizeof(buffer));

        size_t line_pos = 0;
        while (line_pos < sizeof(buffer) - 1)
        {
            int r = SSL_read(ctx->_ssl, &buffer[line_pos], 1);
            if (r <= 0)
            {
                running = 0;
                break;
            }
            if (buffer[line_pos] == '\n')
            {
                buffer[line_pos] = '\0';
                break;
            }
            line_pos++;
        }

        if (!running || line_pos == 0)
        {
            break;
        }

        char command[32] = {0};
        char channel[MAX_CHANNEL_NAME] = {0};
        size_t content_len = 0;

        if (sscanf(buffer, "%31s %255s %zu", command, channel, &content_len)
            >= 2)
        {
            if (strcmp(command, "AUTH") == 0)
            {
                _handle_auth(ctx, channel);
            }
            else if (!ctx->_authenticated)
            {
                _send_response(ctx->_ssl, "ERR Authentication required\n");
            }
            else if (strcmp(command, "SUBSCRIBE") == 0)
            {
                _handle_subscribe(ctx, channel);
            }
            else if (strcmp(command, "PUBLISH") == 0 && content_len > 0)
            {
                _handle_publish(ctx, channel, content_len);
            }
            else if (strcmp(command, "ATTACH") == 0)
            {
                uint64_t sub_id = strtoull(channel, NULL, 10);
                _handle_attach(ctx, sub_id);
            }
            else if (strcmp(command, "QUIT") == 0)
            {
                _send_response(ctx->_ssl, "BYE\n");
                running = 0;
            }
            else
            {
                _send_response(ctx->_ssl, "ERR Unknown command\n");
            }
        }
        else if (strcmp(buffer, "DETACH") == 0)
        {
            if (!ctx->_authenticated)
            {
                _send_response(ctx->_ssl, "ERR Authentication required\n");
            }
            else
            {
                _handle_detach(ctx);
            }
        }
        else if (strcmp(buffer, "QUIT") == 0)
        {
            _send_response(ctx->_ssl, "BYE\n");
            running = 0;
        }
        else
        {
            _send_response(ctx->_ssl, "ERR Invalid command format\n");
        }
    }

    atomic_store(&ctx->_active, 0);

    if (ctx->_subscription && !ctx->_detached)
    {
        pthread_join(ctx->_receiver_thread, NULL);
        subscription_unsubscribe(ctx->_subscription);
        subscription_free(ctx->_subscription);
    }

    SSL_shutdown(ctx->_ssl);
    SSL_free(ctx->_ssl);
    close(ctx->_client_fd);
    free(ctx);

    return NULL;
}

static void*
_accept_thread(void* arg)
{

    struct network_server_t* self = (struct network_server_t*) arg;

    while (atomic_load(&self->_running))
    {

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(self->_server_fd,
                               (struct sockaddr*) &client_addr, &client_len);
        if (client_fd < 0)
        {

            if (atomic_load(&self->_running))
            {
                perror("[network_server] accept");
            }

            continue;
        }

        SSL* ssl = SSL_new(self->_ssl_ctx);
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) <= 0)
        {

            _log_ssl_error("SSL handshake failed");
            SSL_free(ssl);
            close(client_fd);

            continue;
        }

        struct client_context_t* ctx = malloc(sizeof(struct client_context_t));
        if (!ctx)
        {

            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);

            continue;
        }

        ctx->_server = self;
        ctx->_ssl = ssl;
        ctx->_client_fd = client_fd;
        ctx->_subscription = NULL;
        ctx->_detached = 0;
        ctx->_authenticated = 0;
        atomic_init(&ctx->_active, 0);

        pthread_t handler_thread;
        if (pthread_create(&handler_thread, NULL, _client_handler, ctx) != 0)
        {

            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_fd);
            free(ctx);

            continue;
        }
        pthread_detach(handler_thread);

        printf("[network_server] Client connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }

    return NULL;
}

int
network_server_new(struct network_server_configuration_t* config,
                   struct network_server_t** out_self)
{

    if (!config || !out_self)
    {
        return 1;
    }

    if (!config->_broker || !config->_cert_file || !config->_key_file)
    {
        return 1;
    }

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    struct network_server_t* self = malloc(sizeof(struct network_server_t));
    if (!self)
    {
        return -1;
    }

    self->_broker = config->_broker;
    self->_port = config->_port;
    self->_max_clients = config->_max_clients > 0 ? config->_max_clients : 10;
    self->_server_fd = -1;
    self->_detached_subscriptions = NULL;
    atomic_init(&self->_running, 0);

    if (config->_api_key)
    {
        size_t key_len = strlen(config->_api_key);
        self->_api_key = malloc(key_len + 1);
        if (!self->_api_key)
        {
            free(self);
            return -1;
        }
        memcpy(self->_api_key, config->_api_key, key_len + 1);
    }
    else
    {
        self->_api_key = NULL;
    }

    if (pthread_mutex_init(&self->_detached_mutex, NULL) != 0)
    {
        free(self->_api_key);
        free(self);
        return -1;
    }

    self->_ssl_ctx = _create_ssl_context(config->_cert_file, config->_key_file);
    if (!self->_ssl_ctx)
    {

        pthread_mutex_destroy(&self->_detached_mutex);
        free(self);
        return -1;
    }

    self->_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (self->_server_fd < 0)
    {

        perror("[network_server] socket");
        SSL_CTX_free(self->_ssl_ctx);
        free(self);

        return -1;
    }

    int opt = 1;
    setsockopt(self->_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =
        config->_host ? inet_addr(config->_host) : INADDR_ANY;
    server_addr.sin_port = htons(config->_port);

    if (bind(self->_server_fd, (struct sockaddr*) &server_addr,
             sizeof(server_addr))
        < 0)
    {

        perror("[network_server] bind");
        close(self->_server_fd);

        SSL_CTX_free(self->_ssl_ctx);
        free(self);

        return -1;
    }

    socklen_t len = sizeof(server_addr);
    if (getsockname(self->_server_fd, (struct sockaddr*) &server_addr, &len)
        == 0)
    {
        self->_port = ntohs(server_addr.sin_port);
    }

    *out_self = self;

    return 0;
}

int
network_server_start(struct network_server_t* self)
{

    if (!self)
    {
        return 1;
    }

    if (listen(self->_server_fd, (int) self->_max_clients) < 0)
    {

        perror("[network_server] listen");
        return -1;
    }

    atomic_store(&self->_running, 1);

    if (pthread_create(&self->_accept_thread, NULL, _accept_thread, self) != 0)
    {

        perror("[network_server] pthread_create");
        atomic_store(&self->_running, 0);

        return -1;
    }

    printf("[network_server] Server started on port %d\n", self->_port);

    return 0;
}

int
network_server_stop(struct network_server_t* self)
{

    if (!self)
    {
        return 1;
    }

    if (!atomic_load(&self->_running))
    {
        return 0;
    }

    atomic_store(&self->_running, 0);

    shutdown(self->_server_fd, SHUT_RDWR);
    close(self->_server_fd);
    self->_server_fd = -1;

    pthread_join(self->_accept_thread, NULL);

    printf("[network_server] Server stopped\n");

    return 0;
}

int
network_server_free(struct network_server_t* self)
{

    if (!self)
    {
        return 1;
    }

    if (atomic_load(&self->_running))
    {
        network_server_stop(self);
    }

    // Clean up detached subscriptions
    struct detached_subscription_t* node = self->_detached_subscriptions;
    while (node)
    {
        struct detached_subscription_t* next = node->_next;
        subscription_unsubscribe(node->_subscription);
        subscription_free(node->_subscription);
        free(node);
        node = next;
    }
    pthread_mutex_destroy(&self->_detached_mutex);

    if (self->_server_fd >= 0)
    {
        close(self->_server_fd);
    }

    if (self->_ssl_ctx)
    {
        SSL_CTX_free(self->_ssl_ctx);
    }

    free(self->_api_key);
    free(self);

    return 0;
}

int
network_server_get_port(struct network_server_t* self, int* out_port)
{

    if (!self)
    {
        return 1;
    }

    if (!out_port)
    {
        return 1;
    }

    *out_port = self->_port;
    return 0;
}

// @todo the network server must be considered as solely network interface to
// test the message_broker functionalities.