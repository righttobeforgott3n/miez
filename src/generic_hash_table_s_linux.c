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

    *out_self =
        (generic_hash_table_s) malloc(sizeof(struct generic_hash_table_s_t));
    if (!(*out_self))
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_new - allocation error on struct "
               "generic_hash_table_s_t *");
#endif

        return -1;
    }

    int exit_code = generic_hash_table_new(hash_function, capacity,
                                           &(*out_self)->_generic_hash_table);
    if (exit_code || !(*out_self)->_generic_hash_table)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_new - generic_hash_table internal error");
#endif

        free(*out_self);
        return -1;
    }

    (*out_self)->_mutexes =
        (pthread_mutex_t*) calloc(capacity, sizeof(pthread_mutex_t));
    if (!(*out_self)->_mutexes)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_new - error in allocating mutexes");
#endif

        generic_hash_table_free((*out_self)->_generic_hash_table);
        free(*out_self);

        return -1;
    }

    size_t i = 0;
    while (i < capacity)
    {

        exit_code = pthread_mutex_init((*out_self)->_mutexes + i, NULL);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            fprintf(stderr,
                    "generic_hash_table_s_new - pthread_mutex_init failed at "
                    "index %zu\n",
                    i);
#endif

            size_t j = 0;
            while (j < i)
            {
                pthread_mutex_destroy((*out_self)->_mutexes + j);
                j++;
            }

            free((*out_self)->_mutexes);
            generic_hash_table_free((*out_self)->_generic_hash_table);
            free(*out_self);

            return -1;
        }

        i++;
    }

    return 0;
}

int
generic_hash_table_s_free(generic_hash_table_s self)
{

    if (!self)
    {
        return 1;
    }

    int exit_code = 0;
    size_t capacity = 0;
    generic_hash_table_get_capacity(self->_generic_hash_table, &capacity);

    if (self->_generic_hash_table)
    {

        exit_code = generic_hash_table_free(self->_generic_hash_table);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            printf("generic_hash_table_s_free - "
                   "generic_hash_table_free internal error");

#endif
            return exit_code;
        }
    }

    if (self->_mutexes)
    {

        size_t i = 0;
        while (i < capacity)
        {

            exit_code = pthread_mutex_destroy(&(*(self->_mutexes + i)));
            if (exit_code)
            {

#ifdef STDIO_DEBUG
                fprintf(stderr,
                        "generic_hash_table_s_free - "
                        "generic_hash_table_free failed in freeing mutex at "
                        "index: %zu - exit code: %d",
                        i, exit_code);

#endif
            }

            i++;
        }
    }

    free(self);

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

    // @todo save hash_function as attribute to avoid this overhead.
    size_t (*hash_function)(void*) = NULL;
    int exit_code = generic_hash_table_get_hash_function(
        self->_generic_hash_table, &(hash_function));
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "generic_hash_table_get_hash_function internal error");
#endif
        return exit_code;
    }

    // @todo save capacity as attribute to avoid this overhead.
    size_t capacity = 0;
    exit_code =
        generic_hash_table_get_capacity(self->_generic_hash_table, &capacity);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "generic_hash_table_get_capacity internal error");
#endif
        return exit_code;
    }

    size_t hash = hash_function(key);
    size_t index = hash % capacity;
    pthread_mutex_t m = *(self->_mutexes + index);

    exit_code = pthread_mutex_lock(&m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_insert - "
               "pthread_mutex_lock internal error");
#endif
        return exit_code;
    }

    exit_code = generic_hash_table_insert(self->_generic_hash_table, key, item, free_item_function,
                 deep_copy_item_function);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "generic_hash_table_s_insert - "
                        "generic_hash_table_insert internal error");
#endif
        return exit_code;
    }

    exit_code = pthread_mutex_unlock(&m);
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

    // @todo save capacity as attribute to avoid this overhead.
    size_t capacity = 0;
    int exit_code =
        generic_hash_table_get_capacity(self->_generic_hash_table, &capacity);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_delete - generic_hash_table_get_capacity "
                "exit code: %d\n",
                exit_code);
#endif

        return exit_code;
    }
    if (!capacity)
    {

#ifdef STDIO_DEBUG
        fprintf(
            stderr,
            "generic_hash_table_s_delete - capacity not valid, equals zero\n");
#endif

        return 2;
    }

    // @todo save hash_function as attribute to avoid this overhead.
    size_t (*hash_function)(void*) = NULL;
    exit_code = generic_hash_table_get_hash_function(self->_generic_hash_table,
                                                     &hash_function);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_delete - "
                "generic_hash_table_get_hash_function exit code: %d\n",
                exit_code);
#endif
        return exit_code;
    }
    if (!hash_function)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "generic_hash_table_s_delete - hash function equals NULL\n");
#endif
        return 2;
    }

    size_t hash = hash_function(key);
    size_t index = hash % capacity;
    pthread_mutex_t m = *(self->_mutexes + index);

    exit_code = pthread_mutex_lock(&m);
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

    exit_code =
        generic_hash_table_delete(self->_generic_hash_table, key);
    if (exit_code)
    {

        int exit_code_2 = pthread_mutex_unlock(&m);
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

    exit_code = pthread_mutex_unlock(&m);
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

// @todo include "generic_hash_table_linux.c" ? It breaks the abstraction but
// accessing directly to the internal structure could be more efficient, some
// measurements must be taken. A better solution could be splitting the
// generic_hash_table header into other two: the one public with no defintions
// at all, the other with the struct definition; the latter will be included by
// this module together with the public one.
// @todo improve logs and standardize the format.