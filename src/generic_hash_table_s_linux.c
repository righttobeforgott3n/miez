#include "generic_hash_table.h"
#include "generic_hash_table_s.h"
#include <pthread.h>
#include <stdlib.h>

// @todo add logs <- check why the macro injection is not working
#define STDIO_DEBUG
#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct generic_hash_table_s_t
{

    generic_hash_table _generic_hash_table;
    size_t (*_hash_function)(void*);
    size_t _capacity;

    pthread_mutex_t* _mutexes;
};

int
generic_hash_table_s_new(size_t (*hash_function)(void*), size_t capacity,
                         generic_hash_table_s* out_self)
{

    if (!out_self || !hash_function || !capacity)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_new - bad parameter");
#endif
        return 1;
    }

    generic_hash_table_s self =
        (generic_hash_table_s) malloc(sizeof(struct generic_hash_table_s_t));
    if (!self)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_new - allocation error on struct "
               "generic_hash_table_s_t *");
#endif
        return -1;
    }

    int exit_code = generic_hash_table_new(hash_function, capacity,
                                           &self->_generic_hash_table);
    if (exit_code || !self->_generic_hash_table)
    {
#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_new - generic_hash_table internal error");
#endif
        free(self);
        return -1;
    }

    self->_capacity = capacity;
    self->_hash_function = hash_function;

    self->_mutexes =
        (pthread_mutex_t*) calloc(capacity, sizeof(pthread_mutex_t));
    if (!self->_mutexes)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_new - error in allocating mutexes");
#endif

        generic_hash_table_free(self->_generic_hash_table);
        free(self);

        return -1;
    }

    for (size_t i = 0; i < capacity; i++)
    {

        exit_code = pthread_mutex_init(self->_mutexes + i, NULL);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr,
                    "generic_hash_table_s_new - pthread_mutex_init failed at "
                    "index %zu\n",
                    i);
#endif

            for (size_t j = 0; j < i; j++)
            {
                pthread_mutex_destroy(self->_mutexes + j);
            }

            free(self->_mutexes);
            generic_hash_table_free(self->_generic_hash_table);
            free(self);

            return -1;
        }
    }

    *out_self = self;

    return 0;
}

int
generic_hash_table_s_free(generic_hash_table_s* self)
{

    if (!self || !(*self))
    {
        return 1;
    }

    int exit_code = 0;

    if ((*self)->_mutexes)
    {
        size_t i = 0;
        while (i < (*self)->_capacity)
        {
            exit_code = pthread_mutex_destroy((*self)->_mutexes + i);
            if (exit_code)
            {
#ifdef STDIO_DEBUG
                fprintf(stderr,
                        "generic_hash_table_s_free - "
                        "pthread_mutex_destroy failed at index: %zu - exit "
                        "code: %d\n",
                        i, exit_code);
#endif
            }
            i++;
        }
        free((*self)->_mutexes);
        (*self)->_mutexes = NULL;
    }

    if ((*self)->_generic_hash_table)
    {
        exit_code = generic_hash_table_free((*self)->_generic_hash_table);
        if (exit_code)
        {
#ifdef STDIO_DEBUG
            fprintf(stderr, "generic_hash_table_s_free - "
                            "generic_hash_table_free internal error\n");
#endif
            return exit_code;
        }
    }

    free(*self);
    *self = NULL;

    return exit_code;
}

int
generic_hash_table_s_insert(generic_hash_table_s self, void* key, void* item,
                            void (*free_item_function)(void*),
                            int (*deep_copy_item_function)(void*, void**))
{

    if (!self || !key || !free_item_function || !deep_copy_item_function)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "bad parameter");
#endif
        return 1;
    }

    int exit_code = 0;
    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_capacity;
    pthread_mutex_t* m = self->_mutexes + index;

    exit_code = pthread_mutex_lock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_insert - "
                        "pthread_mutex_lock internal error");
#endif

        return exit_code;
    }

    generic_hash_table_delete(self->_generic_hash_table, key);

    exit_code =
        generic_hash_table_insert(self->_generic_hash_table, key, item,
                                  free_item_function, deep_copy_item_function);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_insert - "
                        "generic_hash_table_insert internal error");
#endif

        int exit_code_2 = pthread_mutex_unlock(m);
        if (exit_code_2)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr,
                    "generic_hash_table_s_insert - "
                    "generic_hash_table_insert on error - pthread_mutex_unlock "
                    "exit code: "
                    "%d\n",
                    exit_code_2);
#endif
            return exit_code_2;
        }

        return exit_code;
    }

    exit_code = pthread_mutex_unlock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_insert - "
                        "pthread_mutex_unlock internal error");
#endif

        return exit_code;
    }

    return 0;
}

int
generic_hash_table_s_delete(generic_hash_table_s self, void* key)
{

    if (!self || !key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_delete - bad parameter\n");
#endif

        return 1;
    }

    int exit_code = 0;
    size_t hash = self->_hash_function(key);
    size_t index = hash % self->_capacity;
    pthread_mutex_t* m = self->_mutexes + index;

    exit_code = pthread_mutex_lock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(
            stderr,
            "generic_hash_table_s_delete - pthread_mutex_lock exit code: %d\n",
            exit_code);
#endif

        return exit_code;
    }

    exit_code = generic_hash_table_delete(self->_generic_hash_table, key);
    if (exit_code)
    {

        int exit_code_2 = pthread_mutex_unlock(m);
        if (exit_code_2)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr,
                    "generic_hash_table_s_delete - "
                    "generic_hash_table_delete on error - pthread_mutex_unlock "
                    "exit code: "
                    "%d\n",
                    exit_code_2);
#endif

            return exit_code_2;
        }

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_delete - generic_hash_table_delete exit "
                "code: %d\n",
                exit_code);
#endif

        return exit_code;
    }

    exit_code = pthread_mutex_unlock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_delete - pthread_mutex_unlock exit code: "
                "%d\n",
                exit_code);
#endif

        return exit_code;
    }

    return 0;
}

// @todo make operations atomic with a transactional-based approach.
// @todo implement hash chaining.
// @todo improve logs and standardize the format.