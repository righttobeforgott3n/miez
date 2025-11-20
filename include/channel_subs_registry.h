#ifndef CHANNEL_SUBS_REGISTRY_H
#define CHANNEL_SUBS_REGISTRY_H

#include <stddef.h>

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 1024
#endif

typedef struct channel_subs_registry_t* channel_subs_registry;

int
channel_subs_registry_new(size_t capacity, channel_subs_registry* out_self);

int
channel_subs_registry_free(channel_subs_registry self);

int
channel_subs_registry_insert(channel_subs_registry self, const char* channel,
                             void* sub);

int
channel_subs_registry_insert_bulk(channel_subs_registry self,
                                  const char* channel, void** subs);

int
channel_subs_registry_delete(channel_subs_registry self, const char* channel);

// @todo the subs structure must be defined.
// @todo a syn. hash table must be implemented and defined.

#endif  // CHANNEL_SUBS_REGISTRY_H