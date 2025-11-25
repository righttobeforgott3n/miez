#include "generic_hash_table_s.h"
#include "test_utils.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct thread_arg_t
{
    size_t thread_id;
    generic_hash_table_s hts;
    char* key;
    int* item;
};

int
copy_int(void* item, void** out)
{

    if (!out)
    {
        return 1;
    }

    int* i = (int*) item;
    int** out_i = (int**) out;

    int* i_copy = (int*) malloc(sizeof(int));
    if (!i_copy)
    {
        return -1;
    }
    *i_copy = *i;

    *out_i = (void*) i_copy;

    return 0;
}

void*
insert(void* arg)
{

    struct thread_arg_t* t_arg = (struct thread_arg_t*) arg;
    generic_hash_table_s hts = t_arg->hts;
    char* key = t_arg->key;
    int* item = t_arg->item;
    size_t thread_id = t_arg->thread_id;

    if (!hts || !key)
    {
        fprintf(stderr, "Thread ID: %zu - insert - bad parameter\n", thread_id);
        return NULL;
    }

    printf("Thread ID: %zu - insert - inserting (key: %s, item: %d)\n",
           thread_id, key, *item);

    int exit_code = generic_hash_table_s_insert(hts, (void*) key, (void*) item,
                                                free, copy_int);
    if (exit_code)
    {
        fprintf(stderr,
                "Thread ID: %zu - insert - generic_hash_table_s_insert exit "
                "code: %d\n",
                thread_id, exit_code);
        return NULL;
    }

    printf("Thread ID: %zu - insert - inserted (key: %s, item: %d)\n",
           thread_id, key, *item);

    return NULL;
}

void*
delete(void* arg)
{

    struct thread_arg_t* t_arg = (struct thread_arg_t*) arg;
    generic_hash_table_s hts = t_arg->hts;
    char* key = t_arg->key;
    size_t thread_id = t_arg->thread_id;

    int exit_code = generic_hash_table_s_delete(hts, (void*) key);
    if (exit_code)
    {
        fprintf(stderr, "Thread ID: %zu - delete - exit code: %d\n", thread_id,
                exit_code);
        return NULL;
    }

    if (t_arg->item)
    {

        printf("Thread ID: %zu - delete - delete entry (key: %s, item: %d)\n",
               thread_id, key, *t_arg->item);
        free(t_arg->item);
    }

    return NULL;
}

void
generic_hash_table_s_new_and_free()
{

    char message[128];

    generic_hash_table_s hts = NULL;
    int exit_code = generic_hash_table_s_new(hash_string, 1000, &hts);
    sprintf(message, "generic_hash_table_s_new - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
    TEST_ASSERT(hts, "Hash Table Syn instance is not NULL");

    // @todo test attributes init.

    exit_code = generic_hash_table_s_free(hts);
    sprintf(message, "generic_hash_table_s_free - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
}

void
generic_hash_table_s_insert_test()
{

    char message[128];

    generic_hash_table_s hts = NULL;
    int exit_code = generic_hash_table_s_new(hash_string, 1000, &hts);
    sprintf(message, "generic_hash_table_s_new - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
    TEST_ASSERT(hts, "Hash Table Syn instance is not NULL");

    char* key_0 = "key_0";
    int value_0 = 33;
    exit_code = generic_hash_table_s_insert(hts, (void*) key_0,
                                            (void*) &value_0, free, copy_int);
    sprintf(message,
            "generic_hash_table_s_insert - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);

    exit_code = generic_hash_table_s_free(hts);
    sprintf(message, "generic_hash_table_s_free - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
}

void
generic_hash_table_s_stress_test(size_t n_threads)
{
    char message[128];

    generic_hash_table_s hts = NULL;
    int exit_code = generic_hash_table_s_new(hash_string, 1000, &hts);
    sprintf(message, "generic_hash_table_s_new - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
    TEST_ASSERT(hts, "Hash Table Syn instance is not NULL");

    char* key_0 = "key_0";
    char* key_1 = "key_1";

    int* items[n_threads];
    struct thread_arg_t* thread_args =
        malloc(n_threads * sizeof(struct thread_arg_t));
    pthread_t threads[n_threads];
    size_t i = 0;
    while (i < n_threads)
    {
        thread_args[i].hts = hts;
        thread_args[i].key = i % 2 == 0 ? key_0 : key_1;
        thread_args[i].thread_id = i;
        items[i] = (void*) malloc(sizeof(int));
        *items[i] = rand() * ((int) i % INT_MAX) + rand();
        thread_args[i].item = items[i];

        pthread_create(&threads[i], NULL, insert, &thread_args[i]);

        i++;
    }

    i = 0;
    while (i < n_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "Generic Hash Table Syn Stress Test is not crashed");

    pthread_t delete_threads[n_threads];
    i = 0;
    while (i < n_threads)
    {
        pthread_create(&delete_threads[i], NULL, delete,
                       (void*) &thread_args[i]);
        i++;
    }

    i = 0;
    while (i < n_threads)
    {
        pthread_join(delete_threads[i], NULL);
        i++;
    }

    free(thread_args);

    exit_code = generic_hash_table_s_free(hts);
    sprintf(message,
            "generic_hash_table_s_stress_test - generic_hash_table_s_free exit "
            "code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    TEST_SUITE("Begin Hash Table Syn Test");

    generic_hash_table_s_new_and_free();
    generic_hash_table_s_insert_test();
    generic_hash_table_s_stress_test(10000);

    TEST_SUITE("End Hash Table Syn Test");

    return 0;
}
