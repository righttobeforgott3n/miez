#include "generic_queue.h"
#include "test_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
generic_queue_new_and_free()
{

    generic_queue q;
    int exit_code = generic_queue_new(0, &q);

    TEST_ASSERT(!exit_code, "Queue is created correctly\n");
    TEST_ASSERT(q, "Queue is not NULL\n");

    exit_code = generic_queue_free(q);
    q = NULL;

    TEST_ASSERT(!exit_code, "Queue is freed correctly\n");
}

int
main(int argc __attribute__((unused)), char** argv __attribute((unused)))
{

    generic_queue_new_and_free();

    return stats.failed;
}