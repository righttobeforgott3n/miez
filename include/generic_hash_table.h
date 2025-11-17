#ifndef GENERIC_HASH_TABLE_H
#define GENERIC_HASH_TABLE_H

#include <stddef.h>

typedef struct generic_hash_table_t *generic_hash_table;

int
generic_hash_table_new(generic_hash_table *self, size_t (*hash)(void *), size_t capacity);

int
generic_hash_table_free(generic_hash_table self);

int
generic_hash_table_insert(generic_hash_table self, void *key, void *item);

int
generic_hash_table_delete(generic_hash_table self, void *key, void **item);

int
generic_hash_table_get(generic_hash_table self, void *key, void **item);

int
generic_hash_table_search(generic_hash_table self, void *item, void **key);

int
generic_hash_table_get_capacity(generic_hash_table self, size_t *capacity);

int
generic_hash_table_get_hash_function(generic_hash_table self, size_t (**hash_function)(void *));

#endif // GENERIC_HASH_TABLE_H