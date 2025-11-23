#include "generic_hash_table.h"
#include <stdlib.h>

struct _hash_table_entry_meta_t
{

    void* _item;
    void (*_free_item_function)(void*);

    size_t* _hash;
};

struct generic_hash_table_t
{

    size_t (*_hash_function)(void*);

    struct _hash_table_entry_meta_t** _buffer;
    size_t _buffer_capacity;
};

int
generic_hash_table_new(size_t (*hash_function)(void*), size_t capacity,
                       generic_hash_table* self_out)
{

    if (!self_out || !hash_function || !capacity)
    {
        return 1;
    }

    *self_out = malloc(sizeof(struct generic_hash_table_t));
    if (!*self_out)
    {
        return -1;
    }

    (*self_out)->_buffer =
        calloc(capacity, sizeof(struct _hash_table_entry_t*));
    if (!(*self_out)->_buffer)
    {

        free(*self_out);
        *self_out = NULL;

        return -1;
    }

    (*self_out)->_hash_function = hash_function;
    (*self_out)->_buffer_capacity = capacity;

    return 0;
}

int
generic_hash_table_free(generic_hash_table self)
{

    if (!self)
    {
        return 1;
    }

    if (self->_buffer)
    {

        size_t i = 0;
        while (i < self->_buffer_capacity)
        {

            struct _hash_table_entry_meta_t* hash_table_entry_meta =
                *(self->_buffer + i);
            if (hash_table_entry_meta)
            {

                if (hash_table_entry_meta->_free_item_function
                    && hash_table_entry_meta->_item)
                {
                    hash_table_entry_meta->_free_item_function(
                        hash_table_entry_meta->_item);
                    hash_table_entry_meta->_item = NULL;
                }
                if (hash_table_entry_meta->_hash)
                {
                    free(hash_table_entry_meta->_hash);
                    hash_table_entry_meta->_hash = NULL;
                }
                free(hash_table_entry_meta);
            }

            i++;
        }

        free(self->_buffer);
    }

    free(self);

    return 0;
}

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* capacity)
{

    if (!self || !capacity)
    {
        return 1;
    }

    *capacity = self->_buffer_capacity;

    return 0;
}

int
generic_hash_table_get_hash_function(generic_hash_table self,
                                     size_t (**hash_function)(void*))
{

    if (!self || !hash_function)
    {
        return 1;
    }

    *hash_function = self->_hash_function;

    return 0;
}

int
generic_hash_table_insert(generic_hash_table self, void* key, void* item,
                          void (*free_item_function)(void*))
{

    if (!self || !key)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t* index = (size_t*) malloc(sizeof(size_t));
    *index = hash % self->_buffer_capacity;

    struct _hash_table_entry_meta_t* hash_table_entry_meta =
        (struct _hash_table_entry_meta_t*) malloc(
            sizeof(struct _hash_table_entry_meta_t));
    if (!hash_table_entry_meta)
    {
        return -1;
    }

    hash_table_entry_meta->_item = item;
    hash_table_entry_meta->_free_item_function = free_item_function;
    hash_table_entry_meta->_hash = index;

    *(self->_buffer + *index) = hash_table_entry_meta;

    return 0;
}

int
generic_hash_table_delete(generic_hash_table self, void* key)
{

    if (!self || !key)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;
    struct _hash_table_entry_meta_t* hash_table_entry_to_delete =
        *(self->_buffer + index);
    if (!hash_table_entry_to_delete)
    {
        return 0;
    }

    if (hash_table_entry_to_delete->_free_item_function)
    {
        hash_table_entry_to_delete->_free_item_function(
            hash_table_entry_to_delete->_item);
        hash_table_entry_to_delete->_item = NULL;
    }
    if (hash_table_entry_to_delete->_hash)
    {
        free(hash_table_entry_to_delete->_hash);
        hash_table_entry_to_delete->_hash = NULL;
    }
    free(hash_table_entry_to_delete);
    *(self->_buffer + index) = NULL;

    return 0;
}

int
generic_hash_table_get(generic_hash_table self, void* key, void** item)
{

    if (!self || !key || !item)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;
    struct _hash_table_entry_meta_t* hash_table_entry_meta =
        *(self->_buffer + index);
    if (!hash_table_entry_meta)
    {
        return 0;
    }

    *item = hash_table_entry_meta->_item;

    return 0;
}

// @todo the tricky test showed that the hash table implementation do not own
// the item completely; I changed my mind and the fully ownership must be
// implemented.
