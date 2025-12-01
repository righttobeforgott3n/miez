#ifndef GENERIC_HASH_TABLE_H
#define GENERIC_HASH_TABLE_H

#include <stddef.h>

typedef struct generic_hash_table_t* generic_hash_table;

// @todo Temporary I have decided to avoid a ownership model configurable hash
// table for simplicity, in future I will improve the structure.
int
generic_hash_table_new(size_t capacity, size_t (*hash_function)(void*),
                       void (*free_value_function)(void*),
                       int (*copy_value_function)(void*, void**),
                       void (*free_key_function)(void*),
                       int (*copy_key_function)(void*, void**),
                       int (*compare_key_function)(void*, void*),
                       generic_hash_table* out_self);

int
generic_hash_table_free(generic_hash_table self);

int
generic_hash_table_get_hash_function(generic_hash_table self,
                                     size_t (**out_hash_function)(void*));

int
generic_hash_table_get_free_value_function(
    generic_hash_table self, void (**out_free_value_function)(void*));

int
generic_hash_table_get_copy_value_function(
    generic_hash_table self, int (**out_copy_value_function)(void*, void**));

int
generic_hash_table_get_free_key_function(generic_hash_table self,
                                         void (**out_free_key_function)(void*));

int
generic_hash_table_get_copy_key_function(generic_hash_table self,
                                         int (**out_copy_key_function)(void*,
                                                                       void**));

int
generic_hash_table_get_compare_key_function(
    generic_hash_table self, int (**out_compare_key_function)(void*, void*));

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* out_capacity);

int
generic_hash_table_get_size(generic_hash_table self, size_t* out_size);

int
generic_hash_table_is_empty(generic_hash_table self);

int
generic_hash_table_insert(generic_hash_table self, void* key, void* value);

int
generic_hash_table_delete(generic_hash_table self, void* key);

int
generic_hash_table_get(generic_hash_table self, void* key, void** out_value);

int
generic_hash_table_contains(generic_hash_table self, void* key);

int
generic_hash_table_clear(generic_hash_table self);

int
generic_hash_table_for_each(generic_hash_table self,
                            void (*apply)(void*, void*));

#endif  // GENERIC_HASH_TABLE_H
