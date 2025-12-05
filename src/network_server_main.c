#define _POSIX_C_SOURCE 200809L

#include "message_broker.h"
#include "network_server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct network_server_t* g_server = NULL;
static struct message_broker_t* g_broker = NULL;

static void
signal_handler(int sig)
{

    (void) sig;
    printf("\n[main] Received signal, shutting down...\n");

    if (g_server)
    {
        network_server_stop(g_server);
    }
}

static void
print_usage(const char* program)
{

    printf("Usage: %s [options]\n", program);
    printf("Options:\n");
    printf("  -p <port>     Port to listen on (default: 8443)\n");
    printf("  -c <cert>     Path to certificate file (default: "
           "certs/server.crt)\n");
    printf("  -k <key>      Path to private key file (default: "
           "certs/server.key)\n");
    printf("  -a <api_key>  API key for authentication (default: none)\n");
    printf("  -t <threads>  Number of broker threads (default: 4)\n");
    printf("  -h            Show this help message\n");
}

int
main(int argc, char** argv)
{

    int port = 8443;
    const char* cert_file = "certs/server.crt";
    const char* key_file = "certs/server.key";
    const char* api_key = NULL;
    size_t n_threads = 4;

    int opt;
    while ((opt = getopt(argc, argv, "p:c:k:a:t:h")) != -1)
    {

        switch (opt)
        {

            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                cert_file = optarg;
                break;
            case 'k':
                key_file = optarg;
                break;
            case 'a':
                api_key = optarg;
                break;
            case 't':
                n_threads = (size_t) atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    printf("========================================\n");
    printf("Message Broker Network Server\n");
    printf("========================================\n");
    printf("Port:        %d\n", port);
    printf("Certificate: %s\n", cert_file);
    printf("Key:         %s\n", key_file);
    printf("API Key:     %s\n", api_key ? "********" : "(none)");
    printf("Threads:     %zu\n", n_threads);
    printf("========================================\n\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    struct message_broker_configuration_t broker_config = {
        ._n_threads = n_threads, ._channels_capacity = 64};

    int exit_code = message_broker_new(&broker_config, &g_broker);
    if (exit_code)
    {
        fprintf(stderr, "Failed to create message broker: %d\n", exit_code);
        return 1;
    }

    struct network_server_configuration_t server_config = {
        ._host = NULL,
        ._port = port,
        ._cert_file = cert_file,
        ._key_file = key_file,
        ._api_key = api_key,
        ._broker = g_broker,
        ._max_clients = 100};

    exit_code = network_server_new(&server_config, &g_server);
    if (exit_code)
    {
        fprintf(stderr, "Failed to create network server: %d\n", exit_code);
        message_broker_free(g_broker);
        return 1;
    }

    exit_code = network_server_start(g_server);
    if (exit_code)
    {
        fprintf(stderr, "Failed to start network server: %d\n", exit_code);
        network_server_free(g_server);
        message_broker_free(g_broker);
        return 1;
    }

    printf("[main] Server running. Press Ctrl+C to stop.\n\n");

    while (1)
    {
        sleep(1);
    }

    network_server_free(g_server);
    message_broker_free(g_broker);

    printf("[main] Shutdown complete.\n");

    return 0;
}
