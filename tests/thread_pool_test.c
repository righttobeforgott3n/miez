#define _POSIX_C_SOURCE 200809L

#include "test_utils.h"
#include "thread_pool.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static atomic_int task_counter = 0;
static atomic_int task_sum = 0;

static void*
increment_task(void* arg)
{
    (void) arg;
    atomic_fetch_add(&task_counter, 1);
    return NULL;
}

static void*
sum_task(void* arg)
{
    int value = *(int*) arg;
    atomic_fetch_add(&task_sum, value);
    return NULL;
}

static void*
slow_task(void* arg)
{
    (void) arg;
    struct timespec ts = {0, 10000000};
    nanosleep(&ts, NULL);
    atomic_fetch_add(&task_counter, 1);
    return NULL;
}

int
thread_pool_new_valid_test()
{
    TEST_SUITE("Thread Pool New Valid Test");

    struct thread_pool_t* pool = NULL;
    int result = thread_pool_new(4, &pool);

    TEST_ASSERT(result == 0, "new should return 0 on success");
    TEST_ASSERT(pool != NULL, "new should set out_self");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_new_null_out_self_test()
{
    TEST_SUITE("Thread Pool New Null Out Self Test");

    int result = thread_pool_new(4, NULL);

    TEST_ASSERT(result == 1, "new should return 1 when out_self is NULL");

    return 0;
}

int
thread_pool_new_zero_threads_test()
{
    TEST_SUITE("Thread Pool New Zero Threads Test");

    struct thread_pool_t* pool = NULL;
    int result = thread_pool_new(0, &pool);

    TEST_ASSERT(result == 1, "new should return 1 when n_threads is 0");
    TEST_ASSERT(pool == NULL, "new should not set out_self on failure");

    return 0;
}

int
thread_pool_free_null_test()
{
    TEST_SUITE("Thread Pool Free Null Test");

    int result = thread_pool_free(NULL);

    TEST_ASSERT(result == 1, "free should return 1 when self is NULL");

    return 0;
}

int
thread_pool_shutdown_null_test()
{
    TEST_SUITE("Thread Pool Shutdown Null Test");

    int result = thread_pool_shutdown(NULL);

    TEST_ASSERT(result == 1, "shutdown should return 1 when self is NULL");

    return 0;
}

int
thread_pool_wait_null_test()
{
    TEST_SUITE("Thread Pool Wait Null Test");

    int result = thread_pool_wait(NULL);

    TEST_ASSERT(result == 1, "wait should return 1 when self is NULL");

    return 0;
}

int
thread_pool_submit_null_self_test()
{
    TEST_SUITE("Thread Pool Submit Null Self Test");

    int result = thread_pool_submit(NULL, increment_task, NULL);

    TEST_ASSERT(result == 1, "submit should return 1 when self is NULL");

    return 0;
}

int
thread_pool_submit_null_function_test()
{
    TEST_SUITE("Thread Pool Submit Null Function Test");

    struct thread_pool_t* pool = NULL;
    thread_pool_new(4, &pool);

    int result = thread_pool_submit(pool, NULL, NULL);

    TEST_ASSERT(result == 1, "submit should return 1 when function is NULL");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_submit_single_task_test()
{
    TEST_SUITE("Thread Pool Submit Single Task Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    exit_code = thread_pool_submit(pool, increment_task, NULL);
    TEST_ASSERT(exit_code == 0, "submit should return 0 on success");

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == 1, "task should have executed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_submit_multiple_tasks_test()
{
    TEST_SUITE("Thread Pool Submit Multiple Tasks Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    const int num_tasks = 100;
    int i = 0;
    while (i < num_tasks)
    {
        exit_code = thread_pool_submit(pool, increment_task, NULL);
        if (exit_code)
        {
            TEST_ASSERT(0, "submit failed");
            break;
        }
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == num_tasks,
                "all tasks should have executed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_submit_with_args_test()
{
    TEST_SUITE("Thread Pool Submit With Args Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_sum, 0);

    int values[10];
    int expected_sum = 0;
    int i = 0;
    while (i < 10)
    {
        values[i] = i + 1;
        expected_sum += values[i];
        exit_code = thread_pool_submit(pool, sum_task, &values[i]);
        if (exit_code)
        {
            TEST_ASSERT(0, "submit failed");
            break;
        }
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_sum) == expected_sum,
                "sum should match expected value");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_wait_test()
{
    TEST_SUITE("Thread Pool Wait Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(2, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    const int num_tasks = 20;
    int i = 0;
    while (i < num_tasks)
    {
        thread_pool_submit(pool, slow_task, NULL);
        i++;
    }

    exit_code = thread_pool_wait(pool);
    TEST_ASSERT(exit_code == 0, "wait should return 0");

    TEST_ASSERT(atomic_load(&task_counter) == num_tasks,
                "all tasks should complete after wait");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_shutdown_test()
{
    TEST_SUITE("Thread Pool Shutdown Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    int i = 0;
    while (i < 10)
    {
        thread_pool_submit(pool, increment_task, NULL);
        i++;
    }

    exit_code = thread_pool_shutdown(pool);
    TEST_ASSERT(exit_code == 0, "shutdown should return 0");

    exit_code = thread_pool_submit(pool, increment_task, NULL);
    TEST_ASSERT(exit_code == 1, "submit should fail after shutdown");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_single_thread_test()
{
    TEST_SUITE("Thread Pool Single Thread Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(1, &pool);
    TEST_ASSERT(exit_code == 0, "pool created with single thread");

    atomic_store(&task_counter, 0);

    const int num_tasks = 50;
    int i = 0;
    while (i < num_tasks)
    {
        thread_pool_submit(pool, increment_task, NULL);
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == num_tasks,
                "all tasks executed with single thread");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_many_threads_test()
{
    TEST_SUITE("Thread Pool Many Threads Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(16, &pool);
    TEST_ASSERT(exit_code == 0, "pool created with 16 threads");

    atomic_store(&task_counter, 0);

    const int num_tasks = 100;
    int i = 0;
    while (i < num_tasks)
    {
        thread_pool_submit(pool, increment_task, NULL);
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == num_tasks,
                "all tasks executed with many threads");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_high_load_test()
{
    TEST_SUITE("Thread Pool High Load Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(8, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    const int num_tasks = 10000;
    int i = 0;
    while (i < num_tasks)
    {
        exit_code = thread_pool_submit(pool, increment_task, NULL);
        if (exit_code)
        {
            TEST_ASSERT(0, "submit failed during high load");
            break;
        }
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == num_tasks,
                "all 10000 tasks executed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_stress_test()
{
    TEST_SUITE("Thread Pool Stress Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    const int num_tasks = 50000;
    int i = 0;
    while (i < num_tasks)
    {
        exit_code = thread_pool_submit(pool, increment_task, NULL);
        if (exit_code)
        {
            break;
        }
        i++;
    }

    thread_pool_wait(pool);

    int final_count = atomic_load(&task_counter);
    printf("  Tasks submitted: %d, Tasks executed: %d\n", i, final_count);

    TEST_ASSERT(final_count == i, "all submitted tasks executed");

    thread_pool_free(pool);

    return 0;
}

typedef struct
{
    struct thread_pool_t* pool;
    int num_tasks;
    atomic_int* submitted;
} producer_args_t;

static void*
producer_thread(void* arg)
{
    producer_args_t* args = (producer_args_t*) arg;
    int local_submitted = 0;

    int i = 0;
    while (i < args->num_tasks)
    {
        if (thread_pool_submit(args->pool, increment_task, NULL) == 0)
        {
            local_submitted++;
        }
        i++;
    }

    atomic_fetch_add(args->submitted, local_submitted);
    return NULL;
}

int
thread_pool_concurrent_submit_test()
{
    TEST_SUITE("Thread Pool Concurrent Submit Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    const int num_producers = 8;
    const int tasks_per_producer = 1000;
    pthread_t producers[8];
    producer_args_t args[8];
    atomic_int submitted = 0;

    int i = 0;
    while (i < num_producers)
    {
        args[i].pool = pool;
        args[i].num_tasks = tasks_per_producer;
        args[i].submitted = &submitted;
        pthread_create(&producers[i], NULL, producer_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_producers)
    {
        pthread_join(producers[i], NULL);
        i++;
    }

    thread_pool_wait(pool);

    int total_submitted = atomic_load(&submitted);
    int total_executed = atomic_load(&task_counter);

    printf("  Submitted: %d, Executed: %d\n", total_submitted, total_executed);

    TEST_ASSERT(total_executed == total_submitted,
                "all submitted tasks executed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_rapid_create_destroy_test()
{
    TEST_SUITE("Thread Pool Rapid Create/Destroy Test");

    const int iterations = 50;
    int success = 1;

    int i = 0;
    while (i < iterations)
    {
        struct thread_pool_t* pool = NULL;
        int exit_code = thread_pool_new(4, &pool);
        if (exit_code != 0 || pool == NULL)
        {
            success = 0;
            break;
        }

        thread_pool_submit(pool, increment_task, NULL);
        thread_pool_free(pool);
        i++;
    }

    TEST_ASSERT(success, "rapid create/destroy should not crash");

    return 0;
}

int
thread_pool_multiple_waits_test()
{
    TEST_SUITE("Thread Pool Multiple Waits Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    int i = 0;
    while (i < 10)
    {
        thread_pool_submit(pool, increment_task, NULL);
        i++;
    }

    thread_pool_wait(pool);
    TEST_ASSERT(atomic_load(&task_counter) == 10, "first wait completed");

    i = 0;
    while (i < 10)
    {
        thread_pool_submit(pool, increment_task, NULL);
        i++;
    }

    thread_pool_wait(pool);
    TEST_ASSERT(atomic_load(&task_counter) == 20, "second wait completed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_empty_wait_test()
{
    TEST_SUITE("Thread Pool Empty Wait Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    exit_code = thread_pool_wait(pool);
    TEST_ASSERT(exit_code == 0, "wait on empty pool should return immediately");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_immediate_shutdown_test()
{
    TEST_SUITE("Thread Pool Immediate Shutdown Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    exit_code = thread_pool_shutdown(pool);
    TEST_ASSERT(exit_code == 0, "immediate shutdown should succeed");

    thread_pool_free(pool);

    return 0;
}

static atomic_int parallel_execution_count = 0;
static atomic_int max_parallel = 0;

static void*
parallel_check_task(void* arg)
{
    (void) arg;
    int current = atomic_fetch_add(&parallel_execution_count, 1) + 1;

    int old_max = atomic_load(&max_parallel);
    while (current > old_max)
    {
        if (atomic_compare_exchange_weak(&max_parallel, &old_max, current))
        {
            break;
        }
    }

    struct timespec ts = {0, 50000000};
    nanosleep(&ts, NULL);

    atomic_fetch_sub(&parallel_execution_count, 1);
    atomic_fetch_add(&task_counter, 1);

    return NULL;
}

int
thread_pool_parallelism_test()
{
    TEST_SUITE("Thread Pool Parallelism Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created with 4 threads");

    atomic_store(&task_counter, 0);
    atomic_store(&parallel_execution_count, 0);
    atomic_store(&max_parallel, 0);

    const int num_tasks = 20;
    int i = 0;
    while (i < num_tasks)
    {
        thread_pool_submit(pool, parallel_check_task, NULL);
        i++;
    }

    thread_pool_wait(pool);

    int observed_max = atomic_load(&max_parallel);
    printf("  Max parallel execution observed: %d\n", observed_max);

    TEST_ASSERT(observed_max > 1, "tasks should execute in parallel");
    TEST_ASSERT(observed_max <= 4,
                "parallelism should not exceed thread count");
    TEST_ASSERT(atomic_load(&task_counter) == num_tasks, "all tasks completed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_task_order_independence_test()
{
    TEST_SUITE("Thread Pool Task Order Independence Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_sum, 0);

    int* values = malloc(sizeof(int) * 100);
    int expected_sum = 0;
    int i = 0;
    while (i < 100)
    {
        values[i] = i + 1;
        expected_sum += values[i];
        thread_pool_submit(pool, sum_task, &values[i]);
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_sum) == expected_sum,
                "sum should be correct regardless of execution order");

    free(values);
    thread_pool_free(pool);

    return 0;
}

int
thread_pool_mixed_task_duration_test()
{
    TEST_SUITE("Thread Pool Mixed Task Duration Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    int i = 0;
    while (i < 50)
    {
        if (i % 5 == 0)
        {
            thread_pool_submit(pool, slow_task, NULL);
        }
        else
        {
            thread_pool_submit(pool, increment_task, NULL);
        }
        i++;
    }

    thread_pool_wait(pool);

    TEST_ASSERT(atomic_load(&task_counter) == 50,
                "all mixed duration tasks completed");

    thread_pool_free(pool);

    return 0;
}

int
thread_pool_data_integrity_test()
{
    TEST_SUITE("Thread Pool Data Integrity Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(8, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_sum, 0);

    const int num_tasks = 1000;
    int* values = malloc(sizeof(int) * num_tasks);
    int expected_sum = 0;

    int i = 0;
    while (i < num_tasks)
    {
        values[i] = i;
        expected_sum += i;
        thread_pool_submit(pool, sum_task, &values[i]);
        i++;
    }

    thread_pool_wait(pool);

    int actual_sum = atomic_load(&task_sum);
    printf("  Expected sum: %d, Actual sum: %d\n", expected_sum, actual_sum);

    TEST_ASSERT(actual_sum == expected_sum, "data integrity maintained");

    free(values);
    thread_pool_free(pool);

    return 0;
}

int
thread_pool_burst_submit_test()
{
    TEST_SUITE("Thread Pool Burst Submit Test");

    struct thread_pool_t* pool = NULL;
    int exit_code = thread_pool_new(4, &pool);
    TEST_ASSERT(exit_code == 0, "pool created");

    atomic_store(&task_counter, 0);

    int burst = 0;
    while (burst < 10)
    {
        int i = 0;
        while (i < 100)
        {
            thread_pool_submit(pool, increment_task, NULL);
            i++;
        }

        thread_pool_wait(pool);
        burst++;
    }

    TEST_ASSERT(atomic_load(&task_counter) == 1000,
                "all burst tasks completed");

    thread_pool_free(pool);

    return 0;
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    printf("\n");
    printf("*****************************************\n");
    printf("Begin Thread Pool Test Suite\n");
    printf("*****************************************\n");

    thread_pool_new_valid_test();
    thread_pool_new_null_out_self_test();
    thread_pool_new_zero_threads_test();

    thread_pool_free_null_test();
    thread_pool_shutdown_null_test();
    thread_pool_wait_null_test();
    thread_pool_submit_null_self_test();
    thread_pool_submit_null_function_test();

    thread_pool_submit_single_task_test();
    thread_pool_submit_multiple_tasks_test();
    thread_pool_submit_with_args_test();

    thread_pool_wait_test();
    thread_pool_shutdown_test();

    thread_pool_single_thread_test();
    thread_pool_many_threads_test();

    thread_pool_high_load_test();
    thread_pool_stress_test();
    thread_pool_concurrent_submit_test();

    thread_pool_rapid_create_destroy_test();
    thread_pool_multiple_waits_test();
    thread_pool_empty_wait_test();
    thread_pool_immediate_shutdown_test();

    thread_pool_parallelism_test();
    thread_pool_task_order_independence_test();
    thread_pool_mixed_task_duration_test();
    thread_pool_data_integrity_test();
    thread_pool_burst_submit_test();

    printf("\n");
    printf("*****************************************\n");
    printf("End Thread Pool Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}
