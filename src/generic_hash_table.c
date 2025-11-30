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
    generic_linked_list* _buckets;
};

int
generic_hash_table_new(size_t (*hash_function)(void*),
                       int (*compare_function)(void*, void*), size_t capacity,
                       generic_hash_table* out_self)
{

    if (!out_self)
    {
        return 1;
    }

    if (!hash_function)
    {
        return 1;
    }

    if (!compare_function)
    {
        return 1;
    }

    if (!capacity)
    {
        return 1;
    }

    struct generic_hash_table_t* self = (struct generic_hash_table_t*) malloc(
        sizeof(struct generic_hash_table_t));
    if (!self)
    {
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
    *out_self = self;

    return exit_code;
}

int
generic_hash_table_free(generic_hash_table self)
{

    if (!self)
    {
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
        return 1;
    }

    int exit_code = 0;

    size_t i = 0;
    while (i < self->_n_buckets)
    {

        exit_code = generic_linked_list_set_free_function(*(self->_buckets + i),
                                                          free_function);
        if (exit_code)
        {
            ;  // @todo log the warning but continue or ...
        }

        i++;
    }

    return 0;
}

int
generic_hash_table_get_free_function(generic_hash_table self,
                                     void (**out_free_function)(void*));

int
generic_hash_table_set_copy_function(generic_hash_table self,
                                     int (*copy_function)(void*, void**));

int
generic_hash_table_get_copy_function(generic_hash_table self,
                                     int (**out_copy_function)(void*, void**));
