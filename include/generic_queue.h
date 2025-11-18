#ifndef GENERIC_QUEUE_H
#define GENERIC_QUEUE_H

#include <stddef.h>

typedef struct generic_queue_t* generic_queue;

int
generic_queue_new(generic_queue* out_self);

int
generic_queue_free(generic_queue self);

int
generic_queue_size(generic_queue queue, size_t* out_size);

int
generic_queue_enqueue(generic_queue queue, void* data);

int
generic_queue_dequeue(generic_queue queue, void** out_data);

#endif  // GENERIC_QUEUE_H