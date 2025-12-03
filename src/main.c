#include "message_broker.h"
#include <stdio.h>

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    printf("Message Broker Test\n");
    printf("===================\n\n");

    struct message_broker_configuration_t broker_config = {._n_threads = 2};

    struct message_broker_t* broker = NULL;
    int exit_code = message_broker_new(&broker_config, &broker);
    if (exit_code)
    {
        printf("Failed to create message broker: %d\n", exit_code);
        return exit_code;
    }

    printf("Publishing messages...\n\n");

    message_broker_publish(broker, "orders", "order-123-created");
    message_broker_publish(broker, "orders", "order-456-updated");
    message_broker_publish(broker, "payments", "payment-789-received");
    message_broker_publish(broker, "notifications", "user-abc-logged-in");
    message_broker_publish(broker, "orders", "order-123-shipped");

    printf("\nWaiting for all tasks to complete...\n\n");

    message_broker_wait(broker);

    printf("\nAll tasks completed. Cleaning up...\n");

    message_broker_free(broker);

    printf("Done.\n");

    return 0;
}
