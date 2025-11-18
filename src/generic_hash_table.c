#include "generic_hash_table.h"
#include <stdlib.h>

struct generic_hash_table_t
{

    size_t (*_hash_function)(void *);

    void **_buffer;
    size_t _buffer_capacity;
};

int
generic_hash_table_new(size_t (*hash_function)(void *), size_t capacity, generic_hash_table *self_out)
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

    (*self_out)->_buffer = calloc(capacity, sizeof(void *));
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
        free(self->_buffer);
    }

    free(self);

    return 0;
}

int
generic_hash_table_get_capacity(generic_hash_table self, size_t *capacity)
{

    if (!self || !capacity)
    {
        return 1;
    }

    *capacity = self->_buffer_capacity;

    return 0;
}

int
generic_hash_table_get_hash_function(generic_hash_table self, size_t (**hash_function)(void *))
{

    if (!self || !hash_function)
    {
        return 1;
    }

    *hash_function = self->_hash_function;

    return 0;
}

int
generic_hash_table_insert(generic_hash_table self, void *key, void *item)
{

    if (!self || !key)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;
    *(self->_buffer + index) = item;

    return 0;
}

int
generic_hash_table_delete(generic_hash_table self, void *key, void **item)
{

    if (!self || !key || !item)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;
    void *to_check = *(self->_buffer + index);

    if (to_check == NULL)
    {
        return 2;
    }
    
    *item = to_check;
    *(self->_buffer + index) = NULL;

    return 0;
}

int
generic_hash_table_get(generic_hash_table self, void *key, void **item)
{

    if (!self || !key || !item)
    {
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;
    void *to_check = *(self->_buffer + index);

    if (to_check == NULL)
    {
        return 2;
    }
    
    *item = to_check;

    return 0;
}