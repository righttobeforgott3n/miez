#include "generic_hash_table.h"
#include "generic_linked_list.h"
#include <stdlib.h>

#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct generic_hash_table_t
{

    size_t (*_hash_function)(void*);
    int (*_compare_function)(void*, void*);
    size_t _n_buckets;
    size_t _size;
    generic_linked_list* _buckets;
    void (*_free_function)(void*);
    int (*_copy_function)(void*, void**);
};

int
generic_hash_table_new(size_t (*hash_function)(void*),
                       int (*compare_function)(void*, void*), size_t capacity,
                       generic_hash_table* out_self)
{

    if (!out_self)
    {
        // @todo add logs.
        return 1;
    }

    if (!hash_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!compare_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!capacity)
    {
        // @todo add logs.
        return 1;
    }

    struct generic_hash_table_t* self = (struct generic_hash_table_t*) malloc(
        sizeof(struct generic_hash_table_t));
    if (!self)
    {
        // @todo add logs.
        return -1;
    }

    self->_buckets =
        (generic_linked_list*) malloc(sizeof(generic_linked_list) * capacity);
    if (!self->_buckets)
    {
        return -1;
    }

    int exit_code = 0;

    size_t i = 0;
    while (i < capacity)
    {

        exit_code = generic_linked_list_new(self->_buckets + i);
        if (exit_code)
        {

            for (size_t j = 0; j < i; j++)
            {
                generic_linked_list_free(*(self->_buckets + j));
            }

            free(self->_buckets);
            free(self);

            return exit_code;
        }

        i++;
    }
    self->_hash_function = hash_function;
    self->_compare_function = compare_function;
    self->_n_buckets = capacity;
    self->_size = 0;
    self->_free_function = NULL;
    self->_copy_function = NULL;
    *out_self = self;

    return exit_code;
}

int
generic_hash_table_free(generic_hash_table self)
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    int first_error = 0;

    size_t i = 0;
    while (i < self->_n_buckets)
    {

        int exit_code = generic_linked_list_free(*(self->_buckets + i));
        if (exit_code && !first_error)
        {
            first_error = exit_code;
        }

        i++;
    }
    free(self->_buckets);
    free(self);

    return first_error;
}

int
generic_hash_table_set_free_function(generic_hash_table self,
                                     void (*free_function)(void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    int first_exit_code = 0;

    self->_free_function = free_function;

    size_t i = 0;
    while (i < self->_n_buckets)
    {

        int exit_code = generic_linked_list_set_free_function(
            *(self->_buckets + i), self->_free_function);
        if (exit_code && !first_exit_code)
        {
            // @todo warning about it.
            first_exit_code = exit_code;
        }

        i++;
    }

    return first_exit_code;
}

int
generic_hash_table_get_free_function(generic_hash_table self,
                                     void (**out_free_function)(void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_free_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_free_function = self->_free_function;

    return 0;
}

int
generic_hash_table_set_copy_function(generic_hash_table self,
                                     int (*copy_function)(void*, void**))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    int first_exit_code = 0;

    self->_copy_function = copy_function;

    size_t i = 0;
    while (i < self->_n_buckets)
    {

        int exit_code = generic_linked_list_set_copy_function(
            self->_buckets[i], self->_copy_function);
        if (exit_code && !first_exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - failed to set copy function on bucket %zu\n",
                    __PRETTY_FUNCTION__, i);
#endif
            first_exit_code = exit_code;
        }

        i++;
    }

    return first_exit_code;
}

int
generic_hash_table_get_copy_function(generic_hash_table self,
                                     int (**out_copy_function)(void*, void**))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_copy_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_copy_function = self->_copy_function;

    return 0;
}

int
generic_hash_table_get_capacity(generic_hash_table self, size_t* out_capacity)
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_capacity)
    {
        // @todo add logs.
        return 1;
    }

    *out_capacity = self->_n_buckets;

    return 0;
}

int
generic_hash_table_get_size(generic_hash_table self, size_t* out_size)
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_size)
    {
        // @todo add logs.
        return 1;
    }

    *out_size = self->_size;

    return 0;
}

int
generic_hash_table_is_empty(generic_hash_table self)
{

    if (!self)
    {
        // @todo add logs.
        return -1;
    }

    return self->_size == 0;
}

int
generic_hash_table_insert(generic_hash_table self, void* key, void* value)
{

    if (!self)
    {
        // @todo logs.
        return 1;
    }

    if (!key)
    {
        // @todo logs.
        return 1;
    }

    // @note value can be NULL? Yes, for the moment but I do not see any problem
    // with it.

    size_t hashed_key = self->_hash_function(key);
    size_t bucket_index = hashed_key % self->_n_buckets;
    generic_linked_list_insert_first(
        *(self->_buckets + bucket_index),
        value);  // @note insert the last item to the head of the list to follow
                 // the temporal paradigm, maybe something better could be done.
    self->_size++;

    return 0;
}

// @todo temporary for the free, set_free_function etc I have decided to warning
// the first error but to continue with the operation; this decision can be
// reverted or modified in future.