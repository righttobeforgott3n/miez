#include "test_utils.h"
#include "generic_queue_s.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

struct thread_args_t
{
    
    size_t thread_id;

    generic_queue_s queue;

    int data;
    unsigned int times;
};

void*
enqueue_thread(void* args)
{

    struct thread_args_t* t_args = (struct thread_args_t*) args;

    generic_queue_s q = t_args->queue;
    if (!q)
    {
        fprintf(stderr, "Queue instance equals to NULL - Thread ID: %zu - Callback: enqueue_thread", t_args->thread_id);
        return NULL;
    }

    unsigned int i = 0;
    while (i < t_args->times)
    {

        int *data = malloc(sizeof(int));
        *data = t_args->data;
        generic_queue_s_enqueue(t_args->queue, (void*) data);
        
        ++i;
    }

    return NULL;
}

void*
dequeue_thread(void* args)
{

    struct thread_args_t* t_args = (struct thread_args_t*) args;

    generic_queue_s q = t_args->queue;
    if (!q)
    {
        fprintf(stderr, "Queue instance equals to NULL - Thread ID: %zu - Callback: dequeue_thread", t_args->thread_id);
        return NULL;
    }

    int r = 0;
    unsigned int i = 0;
    while (i < t_args->times)
    {

        void *data = NULL;
        r = generic_queue_s_dequeue(t_args->queue, &data);        
        
        if (r)
        {
            fprintf(stderr, "Error in dequeueing the Queue instance - Thread ID: %zu - Callback: dequeue_thread", t_args->thread_id);
            return NULL;
        }

        if (data)
        {
            free(data);
        }

        ++i;
    }

    return NULL;
}

void *
clear_queue_thread(void *args)
{

    struct thread_args_t* t_args = (struct thread_args_t*) args;

    size_t thread_id = t_args->thread_id;

    printf("Thread ID: %zu - Callback: clear_queue_thread - Begin\n", thread_id);

    generic_queue_s q = t_args->queue;
    if (!q)
    {
        fprintf(stderr, "Queue instance equals to NULL - Thread ID: %zu - Callback: clear_queue_thread", thread_id);
        return NULL;
    }

    size_t queue_size = 0;
    while ((queue_size = generic_queue_s_size(q)))
    {

        void *data = NULL;
        int r = generic_queue_s_dequeue(q, &data);

        if (r)
        {
            fprintf(stderr, "Error occured while dequeueing from the Queue Syn instance\n");
            return NULL;
        }

        if (data)
        {
            free(data);
        }
    }

    printf("Thread ID: %zu - Callback: clear_queue_thread - End\n", thread_id);
    return NULL;
}

void
queue_s_concurrent_stress(int n_threads, int n_times_x_thread)
{

    TEST_SUITE("Concurrent Stress Test");

    generic_queue_s q = generic_queue_s_new();
    TEST_ASSERT(q != NULL, "Queue is not NULL");

    int i = 0;
    struct thread_args_t thread_args[n_threads];
    pthread_t threads[n_threads];
    while (i < n_threads)
    {

        thread_args[i].times = n_times_x_thread;
        thread_args[i].queue = q;
        thread_args[i].thread_id = i;
        thread_args[i].data = rand() * i + rand();

        void *(*f)(void *) = NULL;
        if (i % 2 == 0)
        {
            f = dequeue_thread;
        }
        else
        {
            f = enqueue_thread;
        }

        pthread_create(&threads[i], NULL, f, &thread_args[i]);

        i++;
    }

    i = 0;
    while (i < n_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }
    
    size_t q_size = generic_queue_s_size(q);
    if (q_size)
    {
        
        char message[32];
        sprintf(message, "Queue is not empty: %zu\n", q_size);
        TEST_ASSERT(q_size, message);
    
        i = 0;
        pthread_t dequeue_threads[n_threads];
        while (i < n_threads)
        {
            pthread_create(&dequeue_threads[i], NULL, clear_queue_thread, &thread_args[i]); 
            i++;
        }

        i = 0;
        while (i < n_threads)
        {
            pthread_join(dequeue_threads[i], NULL);
            i++;
        }

        size_t q_size_updated = generic_queue_s_size(q); 
        TEST_ASSERT(!q_size_updated, "Queue is empty\n");
    }
    else
    {
        TEST_ASSERT(!q_size, "Queue is empty.\n");
    }

    TEST_ASSERT(1, "Queue survived concurrent stress test without crashing");
    generic_queue_s_free(q);
}

int
main(int argc __attribute__((unused)), char **argv __attribute((unused)))
{

    printf("\n");
    printf("*****************************************\n");
    printf("Begin Thread-Safe Queue Stress Test Suite\n");
    printf("*****************************************\n");

    int n_threads = 100;
    int n_times_x_thread = 10;
    queue_s_concurrent_stress(n_threads, n_times_x_thread);

    printf("\n");
    printf("*****************************************\n");
    printf("End Thread-Safe Queue Stress Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}