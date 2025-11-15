#ifndef PSEUDO_GENERIC_QUEUE_H
#define PSEUDO_GENERIC_QUEUE_H

#include <stddef.h>

typedef struct pseudo_generic_queue_t* pseudo_generic_queue;
pseudo_generic_queue pseudo_generic_queue_new();
void pseudo_generic_queue_free(pseudo_generic_queue queue);
size_t pseudo_generic_queue_size(pseudo_generic_queue queue);
int pseudo_generic_queue_enqueue(pseudo_generic_queue queue, void* data);
int pseudo_generic_queue_dequeue(pseudo_generic_queue queue, void** data);

#endif // PSEUDO_GENERIC_QUEUE_H