#include "generic_hash_table.h"
#include "generic_linked_list.h"
#include <stdlib.h>

#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct generic_hash_table_t
{

    size_t _capacity;
    size_t _size;
    generic_linked_list* _buckets;

    size_t (*_hash_function)(void*);

    void (*_free_value_function)(void*);
    int (*_copy_value_function)(void*, void**);

    void (*_free_key_function)(void*);
    int (*_copy_key_function)(void*, void**);
    int (*_compare_key_function)(void*, void*);
};

struct _key_value_t
{

    void* _key;
    void* _value;
};

int
generic_hash_table_new(size_t capacity, size_t (*hash_function)(void*),
                       void (*free_value_function)(void*),
                       int (*copy_value_function)(void*, void**),
                       void (*free_key_function)(void*),
                       int (*copy_key_function)(void*, void**),
                       int (*compare_key_function)(void*, void*),
                       generic_hash_table* out_self)
{

    if (!out_self)
    {
        // @todo add logs.
        return 1;
    }

    if (!free_value_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!copy_value_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!free_key_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!copy_key_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!compare_key_function)
    {
        // @todo add logs.
        return 1;
    }

    if (!capacity)
    {
        // @todo add logs.
        return 1;
    }

    if (!hash_function)
    {
        // @todo log.
        return 1;
    }

    struct generic_hash_table_t* self = (struct generic_hash_table_t*) malloc(
        sizeof(struct generic_hash_table_t));
    if (!self)
    {
        // @todo add logs.
        return -1;
    }

    self->_hash_function = hash_function;
    self->_compare_key_function = compare_key_function;
    self->_free_value_function = free_value_function;
    self->_copy_value_function = copy_value_function;
    self->_free_key_function = free_key_function;
    self->_copy_key_function = copy_key_function;
    self->_capacity = capacity;
    self->_size = 0;
    self->_buckets =
        (generic_linked_list*) malloc(sizeof(generic_linked_list) * capacity);
    if (!self->_buckets)
    {
        return -1;
    }
    size_t i = 0;
    while (i < capacity)
    {

        int exit_code = generic_linked_list_new(
            self->_buckets
            + i);  // @note no ownership, the single generic_linked_list will
                   // track only pointers.
        if (exit_code)
        {

            for (size_t j = 0; j < i; j++)
            {
                generic_linked_list_free(*(self->_buckets + j));
            }

            free(self->_buckets);
            free(self);

            return 1;
        }

        i++;
    }

    *out_self = self;

    return 0;
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
    while (i < self->_capacity)
    {

        // @todo apply a for-each with the self->_free_key_value_function.

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
generic_hash_table_get_hash_function(generic_hash_table self,
                                     size_t (**out_hash_function)(void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_hash_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_hash_function = self->_hash_function;

    return 0;
}

int
generic_hash_table_get_free_value_function(
    generic_hash_table self, void (**out_free_value_function)(void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_free_value_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_free_value_function = self->_free_value_function;

    return 0;
}

int
generic_hash_table_get_copy_value_function(
    generic_hash_table self, int (**out_copy_value_function)(void*, void**))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_copy_value_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_copy_value_function = self->_copy_value_function;

    return 0;
}

int
generic_hash_table_get_free_key_function(generic_hash_table self,
                                         void (**out_free_key_function)(void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_free_key_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_free_key_function = self->_free_key_function;

    return 0;
}

int
generic_hash_table_get_copy_key_function(generic_hash_table self,
                                         int (**out_copy_key_function)(void*,
                                                                       void**))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_copy_key_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_copy_key_function = self->_copy_key_function;

    return 0;
}

int
generic_hash_table_get_compare_key_function(
    generic_hash_table self, int (**out_compare_key_function)(void*, void*))
{

    if (!self)
    {
        // @todo add logs.
        return 1;
    }

    if (!out_compare_key_function)
    {
        // @todo add logs.
        return 1;
    }

    *out_compare_key_function = self->_compare_key_function;

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

    *out_capacity = self->_capacity;

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

// @todo I need to insert the key-value pair, not only the value.
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
    size_t bucket_index = hashed_key % self->_capacity;

    struct _key_value_t* pair =
        (struct _key_value_t*) malloc(sizeof(struct _key_value_t));
    if (!pair)
    {
        // @todo log.
        return -1;
    }

    int exit_code = self->_copy_key_function(key, &(pair->_key));  // @todo
    if (exit_code)
    {

        // @todo log
        free(pair);

        return exit_code;
    }

    exit_code = self->_copy_value_function(value, &(pair->_value));
    if (exit_code)
    {

        // @todo log

        self->_free_key_function(pair->_key);
        free(pair);

        return exit_code;
    }

    generic_linked_list_insert_first(
        *(self->_buckets + bucket_index),
        pair);  // @note insert the last item to the head of the list to follow
                // the temporal paradigm, maybe something better could be done.
    self->_size++;

    return 0;
}

// @todo implement the check on exit_code over the iterator calls.
// @todo improve it by buffering the iterator?
int
generic_hash_table_get(generic_hash_table self, void* key, void** out_value)
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

    if (!out_value)
    {
        // @todo logs.
        return 1;
    }

    size_t hashed_key = self->_hash_function(key);
    size_t bucket_index = hashed_key % self->_capacity;

    generic_linked_list_iterator begin = NULL;
    int exit_code = generic_linked_list_iterator_begin(
        *(self->_buckets + bucket_index), &begin);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - failed to create begin iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return exit_code;
    }

    while (generic_linked_list_iterator_is_valid(begin) == 0)
    {

        struct _key_value_t* pair = NULL;
        exit_code = generic_linked_list_iterator_get(begin, (void**) &pair);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - failed to get iterator value\n",
                    __PRETTY_FUNCTION__);
#endif

            generic_linked_list_iterator_free(begin);

            return exit_code;
        }

        if (self->_compare_key_function(key, pair->_key) == 0)
        {

            *out_value = pair->_value;
            generic_linked_list_iterator_free(begin);

            return 0;
        }

        generic_linked_list_iterator_next(begin);
    }
    generic_linked_list_iterator_free(begin);

    *out_value = NULL;

    return 1;
}

// @todo improve it by buffering the iterator?
// @todo implement the check on exit_code over the iterator calls.
int
generic_hash_table_delete(generic_hash_table self, void* key)
{

    if (!self)
    {
        // @todo log
        return 1;
    }

    if (!key)
    {
        // @todo log
        return 1;
    }

    size_t hashed_key = self->_hash_function(key);
    size_t bucket_index = hashed_key % self->_capacity;

    generic_linked_list_iterator begin = NULL;
    int exit_code = generic_linked_list_iterator_begin(
        *(self->_buckets + bucket_index), &begin);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - failed to create begin iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return exit_code;
    }

    while (generic_linked_list_iterator_is_valid(begin) == 0)
    {

        struct _key_value_t* pair = NULL;
        exit_code = generic_linked_list_iterator_get(begin, (void**) &pair);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - failed to get iterator value\n",
                    __PRETTY_FUNCTION__);
#endif

            generic_linked_list_iterator_free(begin);

            return exit_code;
        }

        if (self->_compare_key_function(key, pair->_key) == 0)
        {

            exit_code =
                generic_linked_list_iterator_remove(begin, (void**) &pair);
            if (exit_code)
            {

#ifdef STDIO_DEBUG
                fprintf(stderr, "%s - failed to remove element\n",
                        __PRETTY_FUNCTION__);
#endif

                generic_linked_list_iterator_free(begin);

                return exit_code;
            }

            self->_free_key_function(pair->_key);
            self->_free_value_function(pair->_value);
            free(pair);

            self->_size--;

            generic_linked_list_iterator_free(begin);

            return 0;
        }

        generic_linked_list_iterator_next(begin);
    }
    generic_linked_list_iterator_free(begin);

    return 1;
}

// @todo improve it by buffering the iterator?
// @todo implement the check on exit_code over the iterator calls.
int
generic_hash_table_contains(generic_hash_table self, void* key)
{

    if (!self)
    {
        // @todo log
        return 1;
    }

    if (!key)
    {
        // @todo log
        return 1;
    }

    size_t hashed_key = self->_hash_function(key);
    size_t bucket_index = hashed_key % self->_capacity;

    generic_linked_list_iterator begin = NULL;
    int exit_code = generic_linked_list_iterator_begin(
        *(self->_buckets + bucket_index), &begin);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "%s - failed to create begin iterator\n",
                __PRETTY_FUNCTION__);
#endif

        return exit_code;
    }

    while (generic_linked_list_iterator_is_valid(begin) == 0)
    {

        struct _key_value_t* pair = NULL;
        exit_code = generic_linked_list_iterator_get(begin, (void**) &pair);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr, "%s - failed to get iterator value\n",
                    __PRETTY_FUNCTION__);
#endif

            generic_linked_list_iterator_free(begin);

            return exit_code;
        }

        if (self->_compare_key_function(key, pair->_key) == 0)
        {
            generic_linked_list_iterator_free(begin);
            return 0;
        }

        generic_linked_list_iterator_next(begin);
    }
    generic_linked_list_iterator_free(begin);

    return 1;
}

// @todo temporary for the free, set_free_function etc I have decided to warning
// the first error but to continue with the operation; this decision can be
// reverted or modified in future.