#ifndef GENERIC_QUEUE_SYN_H
#define GENERIC_QUEUE_SYN_H

#include <stddef.h>

typedef struct generic_queue_syn_t* generic_queue_syn;

int
generic_queue_syn_new(generic_queue_syn* out_self);

int
generic_queue_syn_free(generic_queue_syn self);

int
generic_queue_syn_set_free_function(generic_queue_syn self,
                                    void (*free_function)(void*));

int
generic_queue_syn_get_free_function(generic_queue_syn self,
                                    void (**out_free_function)(void*));

int
generic_queue_syn_set_copy_function(generic_queue_syn self,
                                    int (*copy_function)(void*, void**));

int
generic_queue_syn_get_copy_function(generic_queue_syn self,
                                    int (**out_copy_function)(void*, void**));

int
generic_queue_syn_size(generic_queue_syn self, size_t* out_size);

int
generic_queue_syn_is_empty(generic_queue_syn self);

int
generic_queue_syn_enqueue(generic_queue_syn self, void* data);

int
generic_queue_syn_dequeue(generic_queue_syn self, void** out_data);

int
generic_queue_syn_peek(generic_queue_syn self, void** out_data);

int
generic_queue_syn_peek_rear(generic_queue_syn self, void** out_data);

int
generic_queue_syn_clear(generic_queue_syn self);

int
generic_queue_syn_contains(generic_queue_syn self, void* target,
                           int (*compare)(void*, void*));

int
generic_queue_syn_for_each(generic_queue_syn self, void (*apply)(void*));

#endif  // GENERIC_QUEUE_syn_SYN_H