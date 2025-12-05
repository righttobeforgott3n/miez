#define _POSIX_C_SOURCE 200809L

#include "message_broker.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static atomic_int g_running = 1;

struct publisher_args_t
{
    struct message_broker_t* broker;
    const char* channel;
    const char* publisher_name;
    int interval_ms;
};

struct subscriber_args_t
{
    struct message_broker_t* broker;
    const char* channel;
    const char* subscriber_name;
    struct subscription_t* subscription;
};

static void
sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

static void*
publisher_thread(void* arg)
{
    struct publisher_args_t* args = (struct publisher_args_t*) arg;
    int message_count = 0;

    printf("[%s] started on channel '%s'\n", args->publisher_name,
           args->channel);

    while (atomic_load(&g_running))
    {
        char content[128];
        snprintf(content, sizeof(content), "message-%d-from-%s", message_count,
                 args->publisher_name);

        int exit_code =
            message_broker_publish(args->broker, args->channel, content);
        if (exit_code == 0)
        {
            printf("[%s] published: %s\n", args->publisher_name, content);
            message_count++;
        }
        else
        {
            printf("[%s] failed to publish\n", args->publisher_name);
        }

        sleep_ms(args->interval_ms);
    }

    printf("[%s] stopped, published %d messages\n", args->publisher_name,
           message_count);

    return NULL;
}

static void*
subscriber_thread(void* arg)
{
    struct subscriber_args_t* args = (struct subscriber_args_t*) arg;
    int received_count = 0;

    int exit_code = message_broker_subscribe(args->broker, args->channel,
                                             &args->subscription);
    if (exit_code)
    {
        printf("[%s] failed to subscribe to '%s'\n", args->subscriber_name,
               args->channel);
        return NULL;
    }

    uint64_t sub_id = 0;
    subscription_get_id(args->subscription, &sub_id);
    printf("[%s] subscribed to '%s' (id: %lu)\n", args->subscriber_name,
           args->channel, (unsigned long) sub_id);

    while (atomic_load(&g_running))
    {
        struct message_t* msg = NULL;

        exit_code = subscription_try_receive(args->subscription, &msg);
        if (exit_code == 0 && msg)
        {
            uint64_t msg_id = 0;
            const char* content = NULL;
            const char* channel = NULL;

            message_get_id(msg, &msg_id);
            message_get_content(msg, &content);
            message_get_channel(msg, &channel);

            printf("[%s] received (id: %lu, channel: %s): %s\n",
                   args->subscriber_name, (unsigned long) msg_id, channel,
                   content);

            message_free(msg);
            received_count++;
        }
        else
        {
            sleep_ms(100);
        }
    }

    size_t pending = 0;
    subscription_get_pending_count(args->subscription, &pending);
    while (pending > 0)
    {
        struct message_t* msg = NULL;
        if (subscription_try_receive(args->subscription, &msg) == 0 && msg)
        {
            uint64_t msg_id = 0;
            const char* content = NULL;

            message_get_id(msg, &msg_id);
            message_get_content(msg, &content);

            printf("[%s] received (id: %lu): %s\n", args->subscriber_name,
                   (unsigned long) msg_id, content);

            message_free(msg);
            received_count++;
        }
        subscription_get_pending_count(args->subscription, &pending);
    }

    printf("[%s] stopped, received %d messages\n", args->subscriber_name,
           received_count);

    return NULL;
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    printf("========================================\n");
    printf("Message Broker Multi-Thread Test\n");
    printf("========================================\n");
    printf("- 2 channels: 'orders' and 'payments'\n");
    printf("- 2 publishers per channel (4 total)\n");
    printf("- 2 subscribers per channel (4 total)\n");
    printf("- Publishers publish every 2 seconds\n");
    printf("- Test runs for 10 seconds\n");
    printf("========================================\n\n");

    struct message_broker_configuration_t config = {._n_threads = 4,
                                                    ._channels_capacity = 64};

    struct message_broker_t* broker = NULL;
    int exit_code = message_broker_new(&config, &broker);
    if (exit_code)
    {
        printf("Failed to create message broker: %d\n", exit_code);
        return exit_code;
    }

    printf("[main] message broker created\n\n");

    pthread_t pub_threads[4];
    pthread_t sub_threads[4];

    struct publisher_args_t pub_args[4] = {
        {broker, "orders", "pub-orders-1", 2000},
        {broker, "orders", "pub-orders-2", 2000},
        {broker, "payments", "pub-payments-1", 2000},
        {broker, "payments", "pub-payments-2", 2000}};

    struct subscriber_args_t sub_args[4] = {
        {broker, "orders", "sub-orders-1", NULL},
        {broker, "orders", "sub-orders-2", NULL},
        {broker, "payments", "sub-payments-1", NULL},
        {broker, "payments", "sub-payments-2", NULL}};

    printf("[main] starting subscriber threads...\n");
    for (int i = 0; i < 4; i++)
    {
        pthread_create(&sub_threads[i], NULL, subscriber_thread, &sub_args[i]);
    }

    sleep_ms(500);

    printf("\n[main] starting publisher threads...\n\n");
    for (int i = 0; i < 4; i++)
    {
        pthread_create(&pub_threads[i], NULL, publisher_thread, &pub_args[i]);
    }

    printf("[main] running for 10 seconds...\n\n");
    sleep(10);

    printf("\n[main] stopping all threads...\n");
    atomic_store(&g_running, 0);

    for (int i = 0; i < 4; i++)
    {
        pthread_join(pub_threads[i], NULL);
    }
    printf("[main] all publishers stopped\n");

    message_broker_wait(broker);
    printf("[main] all pending messages delivered\n");

    for (int i = 0; i < 4; i++)
    {
        if (sub_args[i].subscription)
        {
            subscription_unsubscribe(sub_args[i].subscription);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        pthread_join(sub_threads[i], NULL);
    }
    printf("[main] all subscribers stopped\n");

    for (int i = 0; i < 4; i++)
    {
        if (sub_args[i].subscription)
        {
            subscription_free(sub_args[i].subscription);
        }
    }

    printf("\n[main] cleaning up...\n");
    message_broker_free(broker);

    printf("[main] done.\n");

    return 0;
}
