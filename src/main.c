#include "message_broker.h"
#include <stdio.h>

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    printf("Message Broker Test\n");
    printf("===================\n\n");

    struct message_broker_configuration_t broker_config = {
        ._n_threads = 2, ._channels_capacity = 64};

    struct message_broker_t* broker = NULL;
    int exit_code = message_broker_new(&broker_config, &broker);
    if (exit_code)
    {
        printf("Failed to create message broker: %d\n", exit_code);
        return exit_code;
    }

    printf("Publishing messages...\n\n");

    message msg = NULL;

    message_new("orders", "order-123-created", &msg);
    message_broker_publish(broker, msg);

    message_new("orders", "order-456-updated", &msg);
    message_broker_publish(broker, msg);

    message_new("payments", "payment-789-received", &msg);
    message_broker_publish(broker, msg);

    message_new("notifications", "user-abc-logged-in", &msg);
    message_broker_publish(broker, msg);

    message_new("orders", "order-123-shipped", &msg);
    message_broker_publish(broker, msg);

    printf("\nWaiting for all tasks to complete...\n\n");
    message_broker_wait(broker);

    printf("\nAll tasks completed. Cleaning up...\n");
    message_broker_free(broker);

    printf("Done.\n");

    return 0;
}
