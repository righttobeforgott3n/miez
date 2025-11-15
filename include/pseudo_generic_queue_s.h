#ifndef PSEUDO_GENERIC_QUEUE_S_H
#define PSEUDO_GENERIC_QUEUE_S_H

#include "pseudo_generic_queue.h"
#include <stddef.h>

typedef struct pseudo_generic_queue_s_t *pseudo_generic_queue_s;

pseudo_generic_queue_s pseudo_generic_queue_s_new();
void pseudo_generic_queue_s_free(pseudo_generic_queue_s queue);
size_t pseudo_generic_queue_s_size(pseudo_generic_queue_s queue);
int pseudo_generic_queue_s_enqueue(pseudo_generic_queue_s queue, void* data);
int pseudo_generic_queue_s_dequeue(pseudo_generic_queue_s queue, void** data);

#endif
