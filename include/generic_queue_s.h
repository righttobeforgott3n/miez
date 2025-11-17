#ifndef GENERIC_QUEUE_S_H
#define GENERIC_QUEUE_S_H

#include "generic_queue.h"
#include <stddef.h>

typedef struct generic_queue_s_t *generic_queue_s;

generic_queue_s
generic_queue_s_new();

void
generic_queue_s_free(generic_queue_s queue);

size_t
generic_queue_s_size(generic_queue_s queue);

int
generic_queue_s_enqueue(generic_queue_s queue, void* data);

int
generic_queue_s_dequeue(generic_queue_s queue, void** data);

#endif // GENERIC_QUEUE_S_H
