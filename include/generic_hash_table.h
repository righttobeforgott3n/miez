#ifndef GENERIC_HASH_TABLE_H
#define GENERIC_HASH_TABLE_H

#include <stddef.h>

typedef struct generic_hash_table_t* generic_hash_table;

int
generic_hash_table_new(size_t (*hash_function)(void*), size_t capacity,
                       generic_hash_table* self_out);

int
generic_hash_table_free(generic_hash_table self);

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* out_capacity);

int
generic_hash_table_get_hash_function(generic_hash_table self,
                                     size_t (**out_hash_function)(void*));

int
generic_hash_table_insert(generic_hash_table self, void* key, void* item,
                          void (*free_item_function)(void*),
                          int (*deep_copy_item_function)(void*, void**));

int
generic_hash_table_get(generic_hash_table self, void* key, void** out_item,
                       int (**out_deep_copy_function)(void*, void**));

int
generic_hash_table_delete(generic_hash_table self, void* key);

// @todo to implement.
// int
// generic_hash_table_search(generic_hash_table self, void* item, void**
// out_key);

#endif  // GENERIC_HASH_TABLE_H