#include "generic_queue_syn.h"
#include "test_utils.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
static int enqueue_count = 0;
static int dequeue_count = 0;

int
copy_int(void* src, void** dst)
{
    if (!src || !dst)
    {
        return 1;
    }

    *dst = malloc(sizeof(int));
    if (!*dst)
    {
        return -1;
    }

    *((int*) *dst) = *((int*) src);

    return 0;
}

void
free_wrapper(void* ptr)
{
    free(ptr);
}

int
compare_int(void* a, void* b)
{
    return *(int*) a - *(int*) b;
}

void
increment_int(void* data)
{
    (*(int*) data)++;
}

/* ==========================================================================
 * Basic Functionality Tests
 * ========================================================================== */

int
generic_queue_syn_new_and_free_test()
{
    TEST_SUITE("Generic Queue Syn New and Free Test");

    generic_queue_syn* q_null = NULL;
    int exit_code = generic_queue_syn_new(q_null);
    TEST_ASSERT(exit_code == -1, "Nullity check passed on creation\n");

    generic_queue_syn q = NULL;
    exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Generic Queue Syn created with success\n");
    TEST_ASSERT(q != NULL, "Queue pointer is not NULL\n");

    size_t size = 1;
    exit_code = generic_queue_syn_size(q, &size);
    TEST_ASSERT(!exit_code && size == 0, "Queue size is zero after creation\n");

    int is_empty = generic_queue_syn_is_empty(q);
    TEST_ASSERT(is_empty == 1, "Queue is empty after creation\n");

    generic_queue_syn q_null_1 = NULL;
    exit_code = generic_queue_syn_free(q_null_1);
    TEST_ASSERT(exit_code == -1, "Nullity check on destruction\n");

    exit_code = generic_queue_syn_free(q);
    TEST_ASSERT(!exit_code, "Generic Queue Syn freed with success\n");

    return 0;
}

int
generic_queue_syn_enqueue_dequeue_test()
{
    TEST_SUITE("Generic Queue Syn Enqueue/Dequeue Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        exit_code = generic_queue_syn_enqueue(q, &values[i]);
        TEST_ASSERT(!exit_code, "Enqueue succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 5, "Size is 5 after enqueues\n");

    void* data = NULL;
    exit_code = generic_queue_syn_dequeue(q, &data);
    TEST_ASSERT(!exit_code, "Dequeue succeeded\n");
    TEST_ASSERT(*(int*) data == 10, "First dequeued element is 10 (FIFO)\n");

    exit_code = generic_queue_syn_dequeue(q, &data);
    TEST_ASSERT(!exit_code, "Dequeue succeeded\n");
    TEST_ASSERT(*(int*) data == 20, "Second dequeued element is 20\n");

    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 3, "Size is 3 after two dequeues\n");

    exit_code = generic_queue_syn_free(q);
    TEST_ASSERT(!exit_code, "Queue freed\n");

    return 0;
}

int
generic_queue_syn_peek_test()
{
    TEST_SUITE("Generic Queue Syn Peek Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    int values[] = {100, 200, 300};
    size_t i = 0;
    while (i < 3)
    {
        generic_queue_syn_enqueue(q, &values[i]);
        i++;
    }

    void* front = NULL;
    void* rear = NULL;

    exit_code = generic_queue_syn_peek(q, &front);
    TEST_ASSERT(!exit_code, "Peek succeeded\n");
    TEST_ASSERT(*(int*) front == 100, "Front element is 100\n");

    exit_code = generic_queue_syn_peek_rear(q, &rear);
    TEST_ASSERT(!exit_code, "Peek rear succeeded\n");
    TEST_ASSERT(*(int*) rear == 300, "Rear element is 300\n");

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 3, "Size unchanged after peeks\n");

    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_clear_test()
{
    TEST_SUITE("Generic Queue Syn Clear Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_copy_function(q, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t i = 0;
    while (i < 100)
    {
        int value = i;
        generic_queue_syn_enqueue(q, &value);
        i++;
    }

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 100, "Size is 100\n");

    exit_code = generic_queue_syn_clear(q);
    TEST_ASSERT(!exit_code, "Clear succeeded\n");

    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 0, "Size is 0 after clear\n");

    int is_empty = generic_queue_syn_is_empty(q);
    TEST_ASSERT(is_empty == 1, "Queue is empty after clear\n");

    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_contains_test()
{
    TEST_SUITE("Generic Queue Syn Contains Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    int values[] = {5, 15, 25, 35, 45};
    size_t i = 0;
    while (i < 5)
    {
        generic_queue_syn_enqueue(q, &values[i]);
        i++;
    }

    int target = 25;
    int contains = generic_queue_syn_contains(q, &target, compare_int);
    TEST_ASSERT(contains == 1, "Queue contains 25\n");

    target = 100;
    contains = generic_queue_syn_contains(q, &target, compare_int);
    TEST_ASSERT(contains == 0, "Queue does not contain 100\n");

    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_for_each_test()
{
    TEST_SUITE("Generic Queue Syn For Each Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    int values[] = {1, 2, 3, 4, 5};
    size_t i = 0;
    while (i < 5)
    {
        generic_queue_syn_enqueue(q, &values[i]);
        i++;
    }

    exit_code = generic_queue_syn_for_each(q, increment_int);
    TEST_ASSERT(!exit_code, "For each succeeded\n");

    TEST_ASSERT(values[0] == 2, "First element incremented to 2\n");
    TEST_ASSERT(values[1] == 3, "Second element incremented to 3\n");
    TEST_ASSERT(values[2] == 4, "Third element incremented to 4\n");
    TEST_ASSERT(values[3] == 5, "Fourth element incremented to 5\n");
    TEST_ASSERT(values[4] == 6, "Fifth element incremented to 6\n");

    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_copy_free_functions_test()
{
    TEST_SUITE("Generic Queue Syn Copy/Free Functions Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    void (*free_fn)(void*) = NULL;
    int (*copy_fn)(void*, void**) = NULL;

    exit_code = generic_queue_syn_get_free_function(q, &free_fn);
    TEST_ASSERT(!exit_code && free_fn == NULL,
                "Free function is NULL by default\n");

    exit_code = generic_queue_syn_get_copy_function(q, &copy_fn);
    TEST_ASSERT(!exit_code && copy_fn == NULL,
                "Copy function is NULL by default\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    exit_code = generic_queue_syn_set_copy_function(q, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_queue_syn_get_free_function(q, &free_fn);
    TEST_ASSERT(!exit_code && free_fn == free_wrapper,
                "Free function retrieved correctly\n");

    exit_code = generic_queue_syn_get_copy_function(q, &copy_fn);
    TEST_ASSERT(!exit_code && copy_fn == copy_int,
                "Copy function retrieved correctly\n");

    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * Concurrent Access Tests
 * ========================================================================== */

typedef struct
{
    generic_queue_syn queue;
    int thread_id;
    int num_operations;
    int* success_count;
    pthread_mutex_t* success_mutex;
} thread_args_t;

void*
producer_thread(void* arg)
{
    thread_args_t* args = (thread_args_t*) arg;
    int local_success = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int* value = malloc(sizeof(int));
        if (value)
        {
            *value = args->thread_id * 100000 + i;
            if (generic_queue_syn_enqueue(args->queue, value) == 0)
            {
                local_success++;
                pthread_mutex_lock(&counter_mutex);
                enqueue_count++;
                pthread_mutex_unlock(&counter_mutex);
            }
            else
            {
                free(value);
            }
        }
        i++;
    }

    pthread_mutex_lock(args->success_mutex);
    *(args->success_count) += local_success;
    pthread_mutex_unlock(args->success_mutex);

    return NULL;
}

void*
consumer_thread(void* arg)
{
    thread_args_t* args = (thread_args_t*) arg;
    int local_success = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        void* data = NULL;
        if (generic_queue_syn_dequeue(args->queue, &data) == 0 && data != NULL)
        {
            free(data);
            local_success++;
            pthread_mutex_lock(&counter_mutex);
            dequeue_count++;
            pthread_mutex_unlock(&counter_mutex);
        }
        i++;
    }

    pthread_mutex_lock(args->success_mutex);
    *(args->success_count) += local_success;
    pthread_mutex_unlock(args->success_mutex);

    return NULL;
}

int
generic_queue_syn_concurrent_producers_test()
{
    TEST_SUITE("Generic Queue Syn Concurrent Producers Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    const int num_threads = 8;
    const int ops_per_thread = 1000;
    pthread_t threads[8];
    thread_args_t args[8];
    int success_count = 0;
    pthread_mutex_t success_mutex = PTHREAD_MUTEX_INITIALIZER;

    enqueue_count = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].queue = q;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].success_count = &success_count;
        args[i].success_mutex = &success_mutex;
        pthread_create(&threads[i], NULL, producer_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == (size_t) (num_threads * ops_per_thread),
                "All items enqueued correctly\n");
    TEST_ASSERT(enqueue_count == num_threads * ops_per_thread,
                "Enqueue count matches\n");

    printf("  Enqueued %d items from %d threads\n", enqueue_count, num_threads);

    pthread_mutex_destroy(&success_mutex);
    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_concurrent_consumers_test()
{
    TEST_SUITE("Generic Queue Syn Concurrent Consumers Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    const int total_items = 10000;
    int i = 0;
    while (i < total_items)
    {
        int* value = malloc(sizeof(int));
        *value = i;
        generic_queue_syn_enqueue(q, value);
        i++;
    }

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == (size_t) total_items, "Queue populated\n");

    const int num_threads = 8;
    const int ops_per_thread = total_items / num_threads + 100;
    pthread_t threads[8];
    thread_args_t args[8];
    int success_count = 0;
    pthread_mutex_t success_mutex = PTHREAD_MUTEX_INITIALIZER;

    dequeue_count = 0;

    i = 0;
    while (i < num_threads)
    {
        args[i].queue = q;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].success_count = &success_count;
        args[i].success_mutex = &success_mutex;
        pthread_create(&threads[i], NULL, consumer_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == 0, "All items consumed\n");
    TEST_ASSERT(dequeue_count == total_items, "Dequeue count matches\n");

    printf("  Dequeued %d items from %d threads\n", dequeue_count, num_threads);

    pthread_mutex_destroy(&success_mutex);
    generic_queue_syn_free(q);
    return 0;
}

int
generic_queue_syn_concurrent_producer_consumer_test()
{
    TEST_SUITE("Generic Queue Syn Concurrent Producer/Consumer Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    const int num_producers = 4;
    const int num_consumers = 4;
    const int ops_per_thread = 2500;

    pthread_t producers[4];
    pthread_t consumers[4];
    thread_args_t producer_args[4];
    thread_args_t consumer_args[4];

    int producer_success = 0;
    int consumer_success = 0;
    pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;

    enqueue_count = 0;
    dequeue_count = 0;

    int i = 0;
    while (i < num_producers)
    {
        producer_args[i].queue = q;
        producer_args[i].thread_id = i;
        producer_args[i].num_operations = ops_per_thread;
        producer_args[i].success_count = &producer_success;
        producer_args[i].success_mutex = &producer_mutex;
        pthread_create(&producers[i], NULL, producer_thread, &producer_args[i]);
        i++;
    }

    i = 0;
    while (i < num_consumers)
    {
        consumer_args[i].queue = q;
        consumer_args[i].thread_id = i;
        consumer_args[i].num_operations = ops_per_thread;
        consumer_args[i].success_count = &consumer_success;
        consumer_args[i].success_mutex = &consumer_mutex;
        pthread_create(&consumers[i], NULL, consumer_thread, &consumer_args[i]);
        i++;
    }

    i = 0;
    while (i < num_producers)
    {
        pthread_join(producers[i], NULL);
        i++;
    }

    i = 0;
    while (i < num_consumers)
    {
        pthread_join(consumers[i], NULL);
        i++;
    }

    size_t remaining = 0;
    generic_queue_syn_size(q, &remaining);

    printf("  Producers enqueued: %d\n", enqueue_count);
    printf("  Consumers dequeued: %d\n", dequeue_count);
    printf("  Remaining in queue: %zu\n", remaining);

    TEST_ASSERT(enqueue_count + (int) remaining >= dequeue_count,
                "Conservation of elements\n");
    TEST_ASSERT(enqueue_count - dequeue_count == (int) remaining,
                "Element count is consistent\n");

    pthread_mutex_destroy(&producer_mutex);
    pthread_mutex_destroy(&consumer_mutex);
    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * High Load Stress Tests
 * ========================================================================== */

typedef struct
{
    generic_queue_syn queue;
    int thread_id;
    int num_operations;
    int* total_enqueued;
    int* total_dequeued;
    pthread_mutex_t* count_mutex;
} stress_thread_args_t;

void*
stress_worker_thread(void* arg)
{
    stress_thread_args_t* args = (stress_thread_args_t*) arg;
    int local_enqueued = 0;
    int local_dequeued = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        /* Alternate between enqueue and dequeue based on iteration */
        if (i % 3 != 0)
        {
            int* value = malloc(sizeof(int));
            if (value)
            {
                *value = args->thread_id * 1000000 + i;
                if (generic_queue_syn_enqueue(args->queue, value) == 0)
                {
                    local_enqueued++;
                }
                else
                {
                    free(value);
                }
            }
        }
        else
        {
            void* data = NULL;
            if (generic_queue_syn_dequeue(args->queue, &data) == 0
                && data != NULL)
            {
                free(data);
                local_dequeued++;
            }
        }
        i++;
    }

    pthread_mutex_lock(args->count_mutex);
    *(args->total_enqueued) += local_enqueued;
    *(args->total_dequeued) += local_dequeued;
    pthread_mutex_unlock(args->count_mutex);

    return NULL;
}

int
generic_queue_syn_high_load_stress_test()
{
    TEST_SUITE("Generic Queue Syn High Load Stress Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    const int num_threads = 16;
    const int ops_per_thread = 5000;
    pthread_t threads[16];
    stress_thread_args_t args[16];

    int total_enqueued = 0;
    int total_dequeued = 0;
    pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

    int i = 0;
    while (i < num_threads)
    {
        args[i].queue = q;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].total_enqueued = &total_enqueued;
        args[i].total_dequeued = &total_dequeued;
        args[i].count_mutex = &count_mutex;
        pthread_create(&threads[i], NULL, stress_worker_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t final_size = 0;
    generic_queue_syn_size(q, &final_size);

    printf("  Threads: %d, Operations per thread: %d\n", num_threads,
           ops_per_thread);
    printf("  Total enqueued: %d\n", total_enqueued);
    printf("  Total dequeued: %d\n", total_dequeued);
    printf("  Final queue size: %zu\n", final_size);

    TEST_ASSERT(total_enqueued - total_dequeued == (int) final_size,
                "Element count is consistent after stress test\n");

    pthread_mutex_destroy(&count_mutex);
    generic_queue_syn_free(q);
    return 0;
}

void*
rapid_enqueue_dequeue_thread(void* arg)
{
    stress_thread_args_t* args = (stress_thread_args_t*) arg;
    int local_enqueued = 0;
    int local_dequeued = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int* value = malloc(sizeof(int));
        if (value)
        {
            *value = i;
            if (generic_queue_syn_enqueue(args->queue, value) == 0)
            {
                local_enqueued++;

                void* data = NULL;
                if (generic_queue_syn_dequeue(args->queue, &data) == 0
                    && data != NULL)
                {
                    free(data);
                    local_dequeued++;
                }
            }
            else
            {
                free(value);
            }
        }
        i++;
    }

    pthread_mutex_lock(args->count_mutex);
    *(args->total_enqueued) += local_enqueued;
    *(args->total_dequeued) += local_dequeued;
    pthread_mutex_unlock(args->count_mutex);

    return NULL;
}

int
generic_queue_syn_rapid_operations_test()
{
    TEST_SUITE("Generic Queue Syn Rapid Enqueue/Dequeue Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    const int num_threads = 8;
    const int ops_per_thread = 10000;
    pthread_t threads[8];
    stress_thread_args_t args[8];

    int total_enqueued = 0;
    int total_dequeued = 0;
    pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

    int i = 0;
    while (i < num_threads)
    {
        args[i].queue = q;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].total_enqueued = &total_enqueued;
        args[i].total_dequeued = &total_dequeued;
        args[i].count_mutex = &count_mutex;
        pthread_create(&threads[i], NULL, rapid_enqueue_dequeue_thread,
                       &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t final_size = 0;
    generic_queue_syn_size(q, &final_size);

    printf("  Total rapid enqueues: %d\n", total_enqueued);
    printf("  Total rapid dequeues: %d\n", total_dequeued);
    printf("  Final queue size: %zu\n", final_size);

    TEST_ASSERT(total_enqueued - total_dequeued == (int) final_size,
                "Element count consistent after rapid operations\n");

    pthread_mutex_destroy(&count_mutex);
    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * Data Integrity Tests
 * ========================================================================== */

typedef struct
{
    generic_queue_syn queue;
    int start_value;
    int count;
    int* success;
    pthread_mutex_t* mutex;
} integrity_args_t;

void*
integrity_producer_thread(void* arg)
{
    integrity_args_t* args = (integrity_args_t*) arg;
    int local_success = 0;

    int i = 0;
    while (i < args->count)
    {
        int* value = malloc(sizeof(int));
        if (value)
        {
            *value = args->start_value + i;
            if (generic_queue_syn_enqueue(args->queue, value) == 0)
            {
                local_success++;
            }
            else
            {
                free(value);
            }
        }
        i++;
    }

    pthread_mutex_lock(args->mutex);
    *(args->success) += local_success;
    pthread_mutex_unlock(args->mutex);

    return NULL;
}

int
generic_queue_syn_data_integrity_test()
{
    TEST_SUITE("Generic Queue Syn Data Integrity Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    const int num_producers = 4;
    const int items_per_producer = 1000;
    pthread_t producers[4];
    integrity_args_t args[4];
    int success_count = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    int i = 0;
    while (i < num_producers)
    {
        args[i].queue = q;
        args[i].start_value = i * items_per_producer;
        args[i].count = items_per_producer;
        args[i].success = &success_count;
        args[i].mutex = &mutex;
        pthread_create(&producers[i], NULL, integrity_producer_thread,
                       &args[i]);
        i++;
    }

    i = 0;
    while (i < num_producers)
    {
        pthread_join(producers[i], NULL);
        i++;
    }

    size_t size = 0;
    generic_queue_syn_size(q, &size);
    TEST_ASSERT(size == (size_t) (num_producers * items_per_producer),
                "All items enqueued\n");

    /* Verify all values are present and unique */
    int* seen = calloc(num_producers * items_per_producer, sizeof(int));
    int duplicates = 0;
    int out_of_range = 0;
    int total_dequeued = 0;

    void* data = NULL;
    while (generic_queue_syn_dequeue(q, &data) == 0 && data != NULL)
    {
        int value = *(int*) data;
        free(data);
        data = NULL;
        total_dequeued++;

        if (value < 0 || value >= num_producers * items_per_producer)
        {
            out_of_range++;
        }
        else if (seen[value])
        {
            duplicates++;
        }
        else
        {
            seen[value] = 1;
        }
    }

    printf("  Total dequeued: %d\n", total_dequeued);
    printf("  Duplicates found: %d\n", duplicates);
    printf("  Out of range values: %d\n", out_of_range);

    TEST_ASSERT(duplicates == 0, "No duplicate values found\n");
    TEST_ASSERT(out_of_range == 0, "No out of range values\n");
    TEST_ASSERT(total_dequeued == num_producers * items_per_producer,
                "All items dequeued\n");

    free(seen);
    pthread_mutex_destroy(&mutex);
    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * Edge Cases Under Concurrency
 * ========================================================================== */

void*
empty_queue_dequeue_thread(void* arg)
{
    generic_queue_syn q = (generic_queue_syn) arg;
    void* data = NULL;

    int i = 0;
    while (i < 100)
    {
        generic_queue_syn_dequeue(q, &data);
        if (data != NULL)
        {
            free(data);
            data = NULL;
        }
        i++;
    }

    return NULL;
}

int
generic_queue_syn_empty_queue_concurrent_test()
{
    TEST_SUITE("Generic Queue Syn Empty Queue Concurrent Dequeue Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    const int num_threads = 8;
    pthread_t threads[8];

    int i = 0;
    while (i < num_threads)
    {
        pthread_create(&threads[i], NULL, empty_queue_dequeue_thread, q);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "No crashes when dequeuing from empty queue concurrently\n");

    int is_empty = generic_queue_syn_is_empty(q);
    TEST_ASSERT(is_empty == 1, "Queue is still empty\n");

    generic_queue_syn_free(q);
    return 0;
}

void*
concurrent_peek_thread(void* arg)
{
    generic_queue_syn q = (generic_queue_syn) arg;
    void* data = NULL;

    int i = 0;
    while (i < 1000)
    {
        generic_queue_syn_peek(q, &data);
        generic_queue_syn_peek_rear(q, &data);
        i++;
    }

    return NULL;
}

void*
concurrent_modify_thread(void* arg)
{
    stress_thread_args_t* args = (stress_thread_args_t*) arg;

    int i = 0;
    while (i < args->num_operations)
    {
        int* value = malloc(sizeof(int));
        if (value)
        {
            *value = i;
            generic_queue_syn_enqueue(args->queue, value);
        }

        void* data = NULL;
        if (generic_queue_syn_dequeue(args->queue, &data) == 0 && data != NULL)
        {
            free(data);
        }
        i++;
    }

    return NULL;
}

int
generic_queue_syn_concurrent_peek_modify_test()
{
    TEST_SUITE("Generic Queue Syn Concurrent Peek While Modifying Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    exit_code = generic_queue_syn_set_free_function(q, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    /* Pre-populate the queue */
    int i = 0;
    while (i < 100)
    {
        int* value = malloc(sizeof(int));
        *value = i;
        generic_queue_syn_enqueue(q, value);
        i++;
    }

    const int num_peek_threads = 4;
    const int num_modify_threads = 4;
    pthread_t peek_threads[4];
    pthread_t modify_threads[4];
    stress_thread_args_t modify_args[4];

    int dummy_enqueued = 0;
    int dummy_dequeued = 0;
    pthread_mutex_t dummy_mutex = PTHREAD_MUTEX_INITIALIZER;

    i = 0;
    while (i < num_peek_threads)
    {
        pthread_create(&peek_threads[i], NULL, concurrent_peek_thread, q);
        i++;
    }

    i = 0;
    while (i < num_modify_threads)
    {
        modify_args[i].queue = q;
        modify_args[i].thread_id = i;
        modify_args[i].num_operations = 500;
        modify_args[i].total_enqueued = &dummy_enqueued;
        modify_args[i].total_dequeued = &dummy_dequeued;
        modify_args[i].count_mutex = &dummy_mutex;
        pthread_create(&modify_threads[i], NULL, concurrent_modify_thread,
                       &modify_args[i]);
        i++;
    }

    i = 0;
    while (i < num_peek_threads)
    {
        pthread_join(peek_threads[i], NULL);
        i++;
    }

    i = 0;
    while (i < num_modify_threads)
    {
        pthread_join(modify_threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "No crashes during concurrent peek and modify operations\n");

    pthread_mutex_destroy(&dummy_mutex);
    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * FIFO Order Verification Under Concurrency
 * ========================================================================== */

int
generic_queue_syn_fifo_order_test()
{
    TEST_SUITE("Generic Queue Syn FIFO Order Test");

    generic_queue_syn q = NULL;
    int exit_code = generic_queue_syn_new(&q);
    TEST_ASSERT(!exit_code, "Queue created\n");

    const int n = 10000;
    int i = 0;
    while (i < n)
    {
        int* value = malloc(sizeof(int));
        *value = i;
        generic_queue_syn_enqueue(q, value);
        i++;
    }

    int in_order = 1;
    int expected = 0;
    void* data = NULL;

    while (generic_queue_syn_dequeue(q, &data) == 0 && data != NULL)
    {
        if (*(int*) data != expected)
        {
            in_order = 0;
        }
        expected++;
        free(data);
        data = NULL;
    }

    TEST_ASSERT(in_order, "FIFO order maintained\n");
    TEST_ASSERT(expected == n, "All elements dequeued\n");

    generic_queue_syn_free(q);
    return 0;
}

/* ==========================================================================
 * Null Parameter Tests
 * ========================================================================== */

int
generic_queue_syn_null_parameter_test()
{
    TEST_SUITE("Generic Queue Syn Null Parameter Test");

    int exit_code;

    exit_code = generic_queue_syn_new(NULL);
    TEST_ASSERT(exit_code == -1, "new with NULL returns -1\n");

    exit_code = generic_queue_syn_free(NULL);
    TEST_ASSERT(exit_code == -1, "free with NULL returns -1\n");

    exit_code = generic_queue_syn_enqueue(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "enqueue with NULL queue returns -1\n");

    exit_code = generic_queue_syn_dequeue(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "dequeue with NULL queue returns -1\n");

    exit_code = generic_queue_syn_peek(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "peek with NULL queue returns -1\n");

    exit_code = generic_queue_syn_peek_rear(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "peek_rear with NULL queue returns -1\n");

    exit_code = generic_queue_syn_size(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "size with NULL queue returns -1\n");

    exit_code = generic_queue_syn_is_empty(NULL);
    TEST_ASSERT(exit_code == -1, "is_empty with NULL queue returns -1\n");

    exit_code = generic_queue_syn_clear(NULL);
    TEST_ASSERT(exit_code == -1, "clear with NULL queue returns -1\n");

    exit_code = generic_queue_syn_contains(NULL, NULL, NULL);
    TEST_ASSERT(exit_code == -1, "contains with NULL queue returns -1\n");

    exit_code = generic_queue_syn_for_each(NULL, NULL);
    TEST_ASSERT(exit_code == -1, "for_each with NULL queue returns -1\n");

    exit_code = generic_queue_syn_set_free_function(NULL, NULL);
    TEST_ASSERT(exit_code == -1,
                "set_free_function with NULL queue returns -1\n");

    exit_code = generic_queue_syn_get_free_function(NULL, NULL);
    TEST_ASSERT(exit_code == -1,
                "get_free_function with NULL queue returns -1\n");

    exit_code = generic_queue_syn_set_copy_function(NULL, NULL);
    TEST_ASSERT(exit_code == -1,
                "set_copy_function with NULL queue returns -1\n");

    exit_code = generic_queue_syn_get_copy_function(NULL, NULL);
    TEST_ASSERT(exit_code == -1,
                "get_copy_function with NULL queue returns -1\n");

    return 0;
}

/* ==========================================================================
 * Main
 * ========================================================================== */

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    printf("\n");
    printf("*************************************************\n");
    printf("Begin Generic Queue Syn (Thread-Safe) Test Suite\n");
    printf("*************************************************\n");

    /* Basic functionality tests */
    generic_queue_syn_new_and_free_test();
    generic_queue_syn_enqueue_dequeue_test();
    generic_queue_syn_peek_test();
    generic_queue_syn_clear_test();
    generic_queue_syn_contains_test();
    generic_queue_syn_for_each_test();
    generic_queue_syn_copy_free_functions_test();
    generic_queue_syn_null_parameter_test();
    generic_queue_syn_fifo_order_test();

    /* Concurrent access tests */
    generic_queue_syn_concurrent_producers_test();
    generic_queue_syn_concurrent_consumers_test();
    generic_queue_syn_concurrent_producer_consumer_test();

    /* High load stress tests */
    generic_queue_syn_high_load_stress_test();
    generic_queue_syn_rapid_operations_test();

    /* Data integrity tests */
    generic_queue_syn_data_integrity_test();

    /* Edge cases under concurrency */
    generic_queue_syn_empty_queue_concurrent_test();
    generic_queue_syn_concurrent_peek_modify_test();

    printf("\n");
    printf("*************************************************\n");
    printf("End Generic Queue Syn (Thread-Safe) Test Suite\n");
    printf("*************************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}
