#include "generic_hash_table.h"
#include "generic_hash_table_s.h"
#include <pthread.h>
#include <stdlib.h>

#ifdef STDIO_DEBUG
#include <stdio.h>
#endif

struct generic_hash_table_s_t
{
    generic_hash_table _generic_hash_table;
    pthread_mutex_t** _mutexes;
};

int
generic_hash_table_s_new(size_t (*hash_function)(void*), size_t capacity,
                         generic_hash_table_s* out_self)
{

    if (!out_self || !hash_function || !capacity)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_new - bad parameter");
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

    (*out_self)->_mutexes = calloc(capacity, sizeof(pthread_mutex_t*));
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

    if (self->_mutexes)
    {

        size_t capacity = 0;
        exit_code = generic_hash_table_get_capacity(self->_generic_hash_table,
                                                    &capacity);
        if (exit_code)
        {

#ifdef STDIO_DEBUG
            printf("generic_hash_table_s_free - "
                   "generic_hash_table_get_capacity internal error");

#endif

            return exit_code;
        }

        size_t i = 0;
        while (i < capacity)
        {

            if (*((self->_mutexes) + i))
            {

                if ((exit_code = pthread_mutex_destroy(*((self->_mutexes) + i)))
                    != 0)
                {

#ifdef STDIO_DEBUG
                    printf("generic_hash_table_s_free - "
                           "pthread_mutex_destroy internal error");

#endif

                    return exit_code;
                }

                free(*((self->_mutexes) + i));
                *((self->_mutexes) + i) = NULL;
            }

            i++;
        }

        free(self->_mutexes);
        self->_mutexes = NULL;
    }

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

    free(self);

    return exit_code;
}

int
generic_hash_table_s_insert(generic_hash_table_s self, void* key, void* item)
{

    if (!self || !key)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "bad parameter");
#endif
        return 1;
    }

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
    pthread_mutex_t* m = *(self->_mutexes + index);
    if (!m)
    {

        m = malloc(sizeof(pthread_mutex_t));
        if (!m)
        {
            return -1;
        }

        exit_code = pthread_mutex_init(m, NULL);
        if (exit_code)
        {
            free(m);
            return -1;
        }

        *(self->_mutexes + index) = m;
    }

    exit_code = pthread_mutex_lock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "pthread_mutex_lock internal error");
#endif
        return exit_code;
    }

    exit_code = generic_hash_table_insert(self->_generic_hash_table, key, item);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "generic_hash_table_insert internal error");
#endif
        return exit_code;
    }

    exit_code = pthread_mutex_unlock(m);
    if (exit_code)
    {

#ifdef STDIO_DEBUG
        printf("generic_hash_table_s_insert - "
               "pthread_mutex_unlock internal error");
#endif
        return exit_code;
    }

    return 0;
}
