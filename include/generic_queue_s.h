#ifndef GENERIC_QUEUE_S_H
#define GENERIC_QUEUE_S_H

#include "generic_queue.h"
#include <stddef.h>

typedef struct generic_queue_s_t* generic_queue_s;

int
generic_queue_s_new(generic_queue_s* out_self);

int
generic_queue_s_free(generic_queue_s self);

int
generic_queue_s_size(generic_queue_s self, size_t* out_size);

int
generic_queue_s_enqueue(generic_queue_s self, void* data);

int
generic_queue_s_dequeue(generic_queue_s self, void** out_data);

#endif  // GENERIC_QUEUE_S_H
