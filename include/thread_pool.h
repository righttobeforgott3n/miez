#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stddef.h>

typedef struct thread_pool_t* thread_pool;

int
thread_pool_new(size_t n_threads, struct thread_pool_t** out_self);

int
thread_pool_free(struct thread_pool_t* self);

int
thread_pool_shutdown(struct thread_pool_t* self);

int
thread_pool_wait(struct thread_pool_t* self);

int
thread_pool_submit(struct thread_pool_t* self, void* (*function)(void*),
                   void* arg);

#endif  // THREAD_POOL_H