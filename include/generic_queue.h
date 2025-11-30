#ifndef GENERIC_QUEUE_H
#define GENERIC_QUEUE_H

#include <stddef.h>

typedef struct generic_queue_t* generic_queue;

int
generic_queue_new(generic_queue* out_self);

int
generic_queue_free(generic_queue self);

int
generic_queue_set_free_function(generic_queue self,
                                void (*free_function)(void*));

int
generic_queue_get_free_function(generic_queue self,
                                void (**out_free_function)(void*));

int
generic_queue_set_copy_function(generic_queue self,
                                int (*copy_function)(void*, void**));

int
generic_queue_get_copy_function(generic_queue self,
                                int (**out_copy_function)(void*, void**));

int
generic_queue_size(generic_queue self, size_t* out_size);

int
generic_queue_is_empty(generic_queue self);

int
generic_queue_enqueue(generic_queue self, void* data);

int
generic_queue_dequeue(generic_queue self, void** out_data);

int
generic_queue_peek(generic_queue self, void** out_data);

int
generic_queue_peek_rear(generic_queue self, void** out_data);

int
generic_queue_clear(generic_queue self);

int
generic_queue_contains(generic_queue self, void* target,
                       int (*compare)(void*, void*));

int
generic_queue_for_each(generic_queue self, void (*apply)(void*));

#endif  // GENERIC_QUEUE_H