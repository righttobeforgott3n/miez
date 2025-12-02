#include "thread_pool.h"
#include <generic_queue_syn.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

struct thread_pool_t
{
    generic_queue_syn _queue_syn;
    pthread_mutex_t* _mutex;
    pthread_cond_t* _new_task;
    pthread_cond_t* _task_done;
    atomic_bool _on;
    atomic_bool _off;
    atomic_size_t _in_flight;
    size_t _n_threads;
    pthread_t* _dispatcher_threads;
};

struct _task_t
{
    void* (*_function)(void*);
    void* _arg;
};

void*
_dispatcher_function(void* arg)
{
    if (!arg)
    {
        return NULL;
    }

    struct thread_pool_t* thread_pool = (struct thread_pool_t*) arg;
    if (!thread_pool)
    {
        return NULL;
    }

    generic_queue_syn queue = thread_pool->_queue_syn;
    if (!queue)
    {
        return NULL;
    }

    pthread_mutex_t* mutex = thread_pool->_mutex;
    if (!mutex)
    {
        return NULL;
    }

    pthread_cond_t* new_task = thread_pool->_new_task;
    if (!new_task)
    {
        return NULL;
    }

    pthread_cond_t* task_done = thread_pool->_task_done;
    if (!task_done)
    {
        return NULL;
    }

    while (1)
    {
        pthread_mutex_lock(mutex);

        while (generic_queue_syn_is_empty(queue)
               && atomic_load(&thread_pool->_on))
        {
            pthread_cond_wait(new_task, mutex);
        }

        if (atomic_load(&thread_pool->_off))
        {
            pthread_mutex_unlock(mutex);
            break;
        }

        if (!atomic_load(&thread_pool->_on)
            && generic_queue_syn_is_empty(queue))
        {
            pthread_mutex_unlock(mutex);
            break;
        }

        pthread_mutex_unlock(mutex);

        struct _task_t* task = NULL;
        int exit_code = generic_queue_syn_dequeue(queue, (void**) &task);
        if (exit_code || !task)
        {
            continue;
        }

        atomic_fetch_add(&thread_pool->_in_flight, 1);

        task->_function(task->_arg);
        free(task);

        atomic_fetch_sub(&thread_pool->_in_flight, 1);

        pthread_mutex_lock(mutex);
        pthread_cond_broadcast(task_done);
        pthread_mutex_unlock(mutex);
    }

    return NULL;
}

int
thread_pool_new(size_t n_threads, struct thread_pool_t** out_self)
{
    if (!out_self)
    {
        return 1;
    }

    if (!n_threads)
    {
        return 1;
    }

    struct thread_pool_t* self =
        (struct thread_pool_t*) malloc(sizeof(struct thread_pool_t));
    if (!self)
    {
        return -1;
    }

    int exit_code = generic_queue_syn_new(&(self->_queue_syn));
    if (exit_code)
    {
        free(self);
        return exit_code;
    }

    self->_mutex = malloc(sizeof(pthread_mutex_t));
    if (!self->_mutex)
    {
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return -1;
    }

    exit_code = pthread_mutex_init(self->_mutex, NULL);
    if (exit_code)
    {
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return exit_code;
    }

    self->_new_task = malloc(sizeof(pthread_cond_t));
    if (!self->_new_task)
    {
        pthread_mutex_destroy(self->_mutex);
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return -1;
    }

    exit_code = pthread_cond_init(self->_new_task, NULL);
    if (exit_code)
    {
        free(self->_new_task);
        pthread_mutex_destroy(self->_mutex);
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return exit_code;
    }

    self->_task_done = malloc(sizeof(pthread_cond_t));
    if (!self->_task_done)
    {
        pthread_cond_destroy(self->_new_task);
        free(self->_new_task);
        pthread_mutex_destroy(self->_mutex);
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return -1;
    }

    exit_code = pthread_cond_init(self->_task_done, NULL);
    if (exit_code)
    {
        free(self->_task_done);
        pthread_cond_destroy(self->_new_task);
        free(self->_new_task);
        pthread_mutex_destroy(self->_mutex);
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return exit_code;
    }

    self->_dispatcher_threads = malloc(sizeof(pthread_t) * n_threads);
    if (!self->_dispatcher_threads)
    {
        pthread_cond_destroy(self->_task_done);
        free(self->_task_done);
        pthread_cond_destroy(self->_new_task);
        free(self->_new_task);
        pthread_mutex_destroy(self->_mutex);
        free(self->_mutex);
        generic_queue_syn_free(self->_queue_syn);
        free(self);
        return -1;
    }

    atomic_init(&self->_on, true);
    atomic_init(&self->_off, false);
    atomic_init(&self->_in_flight, 0);

    self->_n_threads = n_threads;
    for (size_t i = 0; i < self->_n_threads; i++)
    {
        exit_code = pthread_create(&self->_dispatcher_threads[i], NULL,
                                   _dispatcher_function, self);
        if (exit_code)
        {
            atomic_store(&self->_on, false);
            atomic_store(&self->_off, true);

            pthread_mutex_lock(self->_mutex);
            pthread_cond_broadcast(self->_new_task);
            pthread_mutex_unlock(self->_mutex);

            for (size_t j = 0; j < i; j++)
            {
                pthread_join(self->_dispatcher_threads[j], NULL);
            }

            free(self->_dispatcher_threads);
            pthread_cond_destroy(self->_task_done);
            free(self->_task_done);
            pthread_cond_destroy(self->_new_task);
            free(self->_new_task);
            pthread_mutex_destroy(self->_mutex);
            free(self->_mutex);
            generic_queue_syn_free(self->_queue_syn);
            free(self);

            return exit_code;
        }
    }

    *out_self = self;

    return 0;
}

int
thread_pool_free(struct thread_pool_t* self)
{
    if (!self)
    {
        return 1;
    }

    if (!atomic_load(&self->_off))
    {
        thread_pool_shutdown(self);
    }

    struct _task_t* task = NULL;
    while (generic_queue_syn_dequeue(self->_queue_syn, (void**) &task) == 0
           && task != NULL)
    {
        free(task);
        task = NULL;
    }

    pthread_mutex_destroy(self->_mutex);
    free(self->_mutex);
    pthread_cond_destroy(self->_new_task);
    free(self->_new_task);
    pthread_cond_destroy(self->_task_done);
    free(self->_task_done);
    generic_queue_syn_free(self->_queue_syn);
    free(self->_dispatcher_threads);
    free(self);

    return 0;
}

int
thread_pool_shutdown(struct thread_pool_t* self)
{
    if (!self)
    {
        return 1;
    }

    atomic_store(&self->_on, false);
    atomic_store(&self->_off, true);

    pthread_mutex_lock(self->_mutex);
    pthread_cond_broadcast(self->_new_task);
    pthread_mutex_unlock(self->_mutex);

    size_t i = 0;
    while (i < self->_n_threads)
    {
        pthread_join(self->_dispatcher_threads[i], NULL);
        i++;
    }

    return 0;
}

int
thread_pool_wait(struct thread_pool_t* self)
{
    if (!self)
    {
        return 1;
    }

    pthread_mutex_lock(self->_mutex);
    while (!generic_queue_syn_is_empty(self->_queue_syn)
           || atomic_load(&self->_in_flight) > 0)
    {
        pthread_cond_wait(self->_task_done, self->_mutex);
    }
    pthread_mutex_unlock(self->_mutex);

    return 0;
}

int
thread_pool_submit(struct thread_pool_t* self, void* (*function)(void*),
                   void* arg)
{
    if (!self)
    {
        return 1;
    }

    if (!function)
    {
        return 1;
    }

    if (atomic_load(&self->_off))
    {
        return 1;
    }

    struct _task_t* task = malloc(sizeof(struct _task_t));
    if (!task)
    {
        return -1;
    }

    task->_function = function;
    task->_arg = arg;

    int result = generic_queue_syn_enqueue(self->_queue_syn, task);
    if (result)
    {
        free(task);
        return result;
    }

    pthread_mutex_lock(self->_mutex);
    pthread_cond_signal(self->_new_task);
    pthread_mutex_unlock(self->_mutex);

    return 0;
}
