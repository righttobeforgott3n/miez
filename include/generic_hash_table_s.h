#ifndef GENERIC_HASH_TABLE_S_H
#define GENERIC_HASH_TABLE_S_H

#include <stddef.h>

typedef struct generic_hash_table_s_t* generic_hash_table_s;

int
generic_hash_table_s_new(size_t (*hash_function)(void*), size_t capacity,
                         generic_hash_table_s* out_self);

int
generic_hash_table_s_free(generic_hash_table_s self);

int
generic_hash_table_s_insert(generic_hash_table_s self, void* key, void* item);

int
generic_hash_table_s_delete(generic_hash_table_s self, void* key,
                            void** out_item);

#endif