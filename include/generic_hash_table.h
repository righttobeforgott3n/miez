#ifndef GENERIC_HASH_TABLE_H
#define GENERIC_HASH_TABLE_H

#include <stddef.h>

typedef struct generic_hash_table_t* generic_hash_table;

int
generic_hash_table_new(size_t (*hash_function)(void*),
                       int (*compare_function)(void*, void*), size_t capacity,
                       generic_hash_table* out_self);

int
generic_hash_table_free(generic_hash_table self);

int
generic_hash_table_set_free_function(generic_hash_table self,
                                     void (*free_function)(void*));

int
generic_hash_table_get_free_function(generic_hash_table self,
                                     void (**out_free_function)(void*));

int
generic_hash_table_set_copy_function(generic_hash_table self,
                                     int (*copy_function)(void*, void**));

int
generic_hash_table_get_copy_function(generic_hash_table self,
                                     int (**out_copy_function)(void*, void**));

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* out_capacity);

int
generic_hash_table_get_size(generic_hash_table self, size_t* out_size);

int
generic_hash_table_is_empty(generic_hash_table self);

int
generic_hash_table_insert(generic_hash_table self, void* key, void* value);

int
generic_hash_table_get(generic_hash_table self, void* key, void** out_value);

int
generic_hash_table_contains(generic_hash_table self, void* key);

int
generic_hash_table_delete(generic_hash_table self, void* key);

int
generic_hash_table_clear(generic_hash_table self);

int
generic_hash_table_for_each(generic_hash_table self,
                            void (*apply)(void*, void*));

#endif  // GENERIC_HASH_TABLE_H
