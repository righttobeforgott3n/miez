#include "channel_subs_registry.h"
#include "generic_hash_table_s.h"
#include <stdlib.h>

#define HASH_SEED 5381
#define HASH_SHIFT 5

size_t
_hash_string(const char* str)
{

    size_t hash = HASH_SEED;
    while (*str)
    {
        hash = ((hash << HASH_SHIFT) + hash) + *str++;
    }

    return hash;
}

struct channel_subs_registry_t
{

    generic_hash_table_s _generic_hash_table_s;
};

int
channel_subs_registry_new(size_t capacity __attribute__((unused)),
                          channel_subs_registry* out_self
                          __attribute__((unused)))
{

    if (!capacity || !out_self)
    {
        return 1;
    }

    return 0;
}