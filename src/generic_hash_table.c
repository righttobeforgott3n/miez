#include "generic_hash_table.h"
#include <stdlib.h>

#define STDIO_DEBUG
#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct _hash_table_entry_meta_t
{
    void* _item;
    void (*_free_item_function)(void*);
    int (*_deep_copy_item_function)(void*, void**);
};

struct generic_hash_table_t
{

    size_t (*_hash_function)(void*);

    struct _hash_table_entry_meta_t* _buffer;
    size_t _buffer_capacity;
};

int
generic_hash_table_new(size_t (*hash_function)(void*), size_t capacity,
                       generic_hash_table* self_out)
{

    if (!self_out)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self_out parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!hash_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - hash_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!capacity)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - capacity parameter 0\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    struct generic_hash_table_t* self = (struct generic_hash_table_t*) malloc(
        sizeof(struct generic_hash_table_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - error during allocation of struct "
                "generic_hash_table_t\n",
                __PRETTY_FUNCTION__);
#endif

        return -1;
    }

    self->_buffer = (struct _hash_table_entry_meta_t*) calloc(
        capacity, sizeof(struct _hash_table_entry_meta_t));
    if (!self->_buffer)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "%s - error during allocation of "
                "internal buffer\n",
                __PRETTY_FUNCTION__);
#endif

        free(self);
        return -1;
    }

    self->_buffer_capacity = capacity;
    self->_hash_function = hash_function;
    *self_out = self;

    return 0;
}

int
generic_hash_table_free(generic_hash_table self)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (self->_buffer)
    {

        size_t i = 0;
        while (i < self->_buffer_capacity)
        {

            struct _hash_table_entry_meta_t* hash_table_entry_meta =
                self->_buffer + i;

            if (hash_table_entry_meta && hash_table_entry_meta->_item)
            {

                if (hash_table_entry_meta->_free_item_function)
                {

                    hash_table_entry_meta->_free_item_function(
                        hash_table_entry_meta->_item);
                    hash_table_entry_meta->_item = NULL;
                }
            }

            i++;
        }

        free(self->_buffer);
    }

    free(self);

    return 0;
}

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* out_capacity)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_capacity)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_capacity parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_capacity = self->_buffer_capacity;

    return 0;
}

int
generic_hash_table_get_hash_function(generic_hash_table self,
                                     size_t (**out_hash_function)(void*))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_hash_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - out_hash_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    *out_hash_function = self->_hash_function;

    return 0;
}

int
generic_hash_table_insert(generic_hash_table self, void* key, void* item,
                          void (*free_item_function)(void*),
                          int (*deep_copy_item_function)(void*, void**))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif
        return 1;
    }

    if (!key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - key parameter NULL\n", __PRETTY_FUNCTION__);
#endif
        return 1;
    }

    if (!free_item_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - free_item_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif
        return 1;
    }

    if (!deep_copy_item_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - deep_copy_item_function parameter NULL\n",
                __PRETTY_FUNCTION__);
#endif
        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;

    void* new_item = NULL;
    int exit_code = deep_copy_item_function(item, &new_item);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - deep_copy_item_function failed with code %d\n",
                __PRETTY_FUNCTION__, exit_code);
#endif

        return exit_code;
    }

    if ((self->_buffer + index)->_item
        && (self->_buffer + index)->_free_item_function)
    {
        (self->_buffer + index)
            ->_free_item_function((self->_buffer + index)->_item);
    }

    (self->_buffer + index)->_item = new_item;
    (self->_buffer + index)->_free_item_function = free_item_function;
    (self->_buffer + index)->_deep_copy_item_function = deep_copy_item_function;

    return 0;
}

int
generic_hash_table_get(generic_hash_table self, void* key, void** out_item,
                       int (**out_deep_copy_function)(void*, void**))
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - key parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_item)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - key parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!out_deep_copy_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - key parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;

    if (!(self->_buffer + index))
    {
        // @todo add STDIO_DEBUG logs.
        return -1;
    }

    *out_item = (self->_buffer + index)->_item;
    *out_deep_copy_function = (self->_buffer + index)->_deep_copy_item_function;

    return 0;
}

int
generic_hash_table_delete(generic_hash_table self, void* key)
{

    if (!self)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - self parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    if (!key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - key parameter NULL\n", __PRETTY_FUNCTION__);
#endif

        return 1;
    }

    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_buffer_capacity;

    if (!(self->_buffer + index))
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - bucket of index %zu NULL\n", __PRETTY_FUNCTION__,
                index);
#endif
        return -1;
    }

    if (!(self->_buffer + index)->_free_item_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - item free_function of bucket index %zu NULL\n",
                __PRETTY_FUNCTION__, index);
#endif
        return -1;
    }

    (self->_buffer + index)
        ->_free_item_function((self->_buffer + index)->_item);
    (self->_buffer + index)->_item = NULL;
    (self->_buffer + index)->_deep_copy_item_function = NULL;
    (self->_buffer + index)->_free_item_function = NULL;

    return 0;
}

// @todo add canary system to both struct generic_hash_table_t and struct
// _hash_table_entry_meta_t
