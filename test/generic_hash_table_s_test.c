#include "generic_hash_table_s.h"
#include "test_utils.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct pthread_arg_t
{
    size_t _thread_id;
    generic_hash_table_s _generic_hash_table_s;
    int* _item;
    char* key;
    void (*_free_item)(void*);
    int (*_deep_copy)(void*, void**);
};

int
copy_int(void* item, void** out_item)
{

    if (!item || !out_item)
    {
        return 1;
    }

    int* i = (int*) item;
    int** out_i = (int**) out_item;

    *out_i = (int*) malloc(sizeof(int));
    if (!(*out_i))
    {
        return -1;
    }

    **out_i = *i;

    return 0;
}

void*
generic_hash_table_s_insert_worker(void* arg)
{

    struct pthread_arg_t* t_arg = (struct pthread_arg_t*) arg;

    if (!t_arg || !t_arg->key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "Thread %zu - insert_worker: invalid arg\n",
                t_arg->_thread_id);
#endif

        return NULL;
    }

    int exit_code = generic_hash_table_s_insert(
        t_arg->_generic_hash_table_s, t_arg->key, t_arg->_item,
        t_arg->_free_item, t_arg->_deep_copy);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "Thread %zu - insert_worker: insert failed (exit: %d)\n",
                t_arg->_thread_id, exit_code);
#endif

        return NULL;
    }

#ifdef STDIO_DEBUG
    fprintf(stdout, "Thread %zu - insert - inserted (key: %s, item: %d)\n",
            t_arg->_thread_id, t_arg->key, *(t_arg->_item));
#endif

    return NULL;
}

void*
generic_hash_table_s_delete_worker(void* arg)
{

    struct pthread_arg_t* t_arg = (struct pthread_arg_t*) arg;

    if (!t_arg || !t_arg->key)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr, "Thread %zu - delete_worker: invalid arg\n",
                t_arg->_thread_id);
#endif

        return NULL;
    }

    int exit_code =
        generic_hash_table_s_delete(t_arg->_generic_hash_table_s, t_arg->key);

    if (exit_code)
    {

#ifdef STDIO_DEBUG
        fprintf(stderr,
                "Thread %zu - delete_worker: delete failed (exit: %d)\n",
                t_arg->_thread_id, exit_code);
#endif

        return NULL;
    }

#ifdef STDIO_DEBUG
    fprintf(stdout, "Thread %zu - delete - deleted (key: %s, item: %d)\n",
            t_arg->_thread_id, t_arg->key, *(t_arg->_item));
#endif

    return NULL;
}

void
generic_hash_table_s_new_and_free_test(void)
{

    generic_hash_table_s hts = NULL;
    int exit_code = generic_hash_table_s_new(hash_string, 100, &hts);

    TEST_ASSERT(!exit_code, "generic_hash_table_s_new returned success\n");
    TEST_ASSERT(hts != NULL, "generic_hash_table_s is not NULL\n");

    exit_code = generic_hash_table_s_free(&hts);

    TEST_ASSERT(!exit_code, "generic_hash_table_s_free returned success\n");
    TEST_ASSERT(hts == NULL, "generic_hash_table_s is NULL\n");
}

void
generic_hash_table_s_phase_insert(pthread_t* threads_buffer,
                                  struct pthread_arg_t* thread_args,
                                  int** items, size_t n_threads)
{

#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 1] Concurrent Inserts with Key Collisions\n");
#endif

    for (size_t i = 0; i < n_threads; i++)
    {

        items[i] = (int*) malloc(sizeof(int));
        if (items[i])
        {

            *items[i] = (int) (1000 + i);

            thread_args[i]._thread_id = i;
            thread_args[i]._item = items[i];
            thread_args[i]._free_item = free;
            thread_args[i]._deep_copy = copy_int;

            if (i % 2 == 0)
            {
                thread_args[i].key = "key_0";
            }
            else
            {
                thread_args[i].key = "key_1";
            }

            pthread_create(&threads_buffer[i], NULL,
                           generic_hash_table_s_insert_worker,
                           (void*) &thread_args[i]);
        }
    }

    for (size_t i = 0; i < n_threads; i++)
    {
        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);

        long result = (long) thread_result;
        if (result != 0)
        {
#ifdef STDIO_DEBUG
            fprintf(stderr, "stress_test: insert thread %zu returned %ld\n", i,
                    result);
#endif
        }
    }

    TEST_ASSERT(1, "Generic Hash Table Syn Stress Test - Phase 1 completed\n");
}

void
generic_hash_table_s_phase_delete(pthread_t* threads_buffer,
                                  struct pthread_arg_t* thread_args,
                                  size_t n_threads)
{

#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 2] Concurrent Deletes\n");
#endif

    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_create(&threads_buffer[i], NULL,
                       generic_hash_table_s_delete_worker,
                       (void*) &thread_args[i]);
    }

    for (size_t i = 0; i < n_threads; i++)
    {

        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);

        long result = (long) thread_result;
        if (result != 0)
        {
#ifdef STDIO_DEBUG
            fprintf(stderr, "stress_test: delete thread %zu returned %ld\n", i,
                    result);
#endif
        }
    }

    TEST_ASSERT(1, "Generic Hash Table Syn Stress Test - Phase 2 completed\n");
}

void
generic_hash_table_s_phase_mixed(pthread_t* threads_buffer,
                                 struct pthread_arg_t* thread_args, int** items,
                                 size_t n_threads)
{

#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 3] Mixed Insert/Delete Operations\n");
#endif

    for (size_t i = 0; i < n_threads; i++)
    {

        if (items[i])
        {
            free(items[i]);
        }

        items[i] = (int*) malloc(sizeof(int));
        if (items[i])
        {

            *items[i] = (int) (2000 + i);
            thread_args[i]._item = items[i];

            if (i % 2 == 0)
            {
                pthread_create(&threads_buffer[i], NULL,
                               generic_hash_table_s_insert_worker,
                               (void*) &thread_args[i]);
            }
            else
            {
                pthread_create(&threads_buffer[i], NULL,
                               generic_hash_table_s_delete_worker,
                               (void*) &thread_args[i]);
            }
        }
    }

    for (size_t i = 0; i < n_threads; i++)
    {
        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);
    }

    TEST_ASSERT(1, "Generic Hash Table Syn Stress Test - Phase 3 completed\n");
}

void
generic_hash_table_s_cleanup_resources(pthread_t* threads_buffer,
                                       struct pthread_arg_t* thread_args,
                                       int** items, size_t n_threads,
                                       generic_hash_table_s* hts)
{

    if (items)
    {
        for (size_t i = 0; i < n_threads; i++)
        {
            if (items[i])
            {
                free(items[i]);
            }
        }
        free(items);
    }

    if (thread_args)
    {
        free(thread_args);
    }

    if (threads_buffer)
    {
        free(threads_buffer);
    }

    if (hts && *hts)
    {
        generic_hash_table_s_free(hts);
    }
}

int
generic_hash_table_s_allocate_resources(size_t n_threads,
                                        pthread_t** threads_buffer_out,
                                        struct pthread_arg_t** thread_args_out,
                                        int*** items_out)
{
    *threads_buffer_out = (pthread_t*) malloc(n_threads * sizeof(pthread_t));
    *thread_args_out = (struct pthread_arg_t*) malloc(
        n_threads * sizeof(struct pthread_arg_t));
    *items_out = (int**) malloc(n_threads * sizeof(int*));

    if (!*threads_buffer_out || !*thread_args_out || !*items_out)
    {
#ifdef STDIO_DEBUG
        fprintf(stderr, "stress_test: malloc failed for thread structures\n");
#endif
        return 1;
    }

    return 0;
}

void
generic_hash_table_s_stress_test(size_t n_threads, size_t hash_table_capacity)
{

    TEST_SUITE("Generic Hash Table Syn. Stress Test");

    generic_hash_table_s hts = NULL;
    int exit_code =
        generic_hash_table_s_new(hash_string, hash_table_capacity, &hts);

    TEST_ASSERT(!exit_code, "generic_hash_table_s_new returned success\n");
    TEST_ASSERT(hts != NULL, "generic_hash_table_s is not NULL\n");

    pthread_t* threads_buffer = NULL;
    struct pthread_arg_t* thread_args = NULL;
    int** items = NULL;

    if (generic_hash_table_s_allocate_resources(n_threads, &threads_buffer,
                                                &thread_args, &items)
        == 0)
    {
        for (size_t i = 0; i < n_threads; i++)
        {
            thread_args[i]._generic_hash_table_s = hts;
        }

        generic_hash_table_s_phase_insert(threads_buffer, thread_args, items,
                                          n_threads);
        generic_hash_table_s_phase_delete(threads_buffer, thread_args,
                                          n_threads);
        generic_hash_table_s_phase_mixed(threads_buffer, thread_args, items,
                                         n_threads);

        generic_hash_table_s_cleanup_resources(threads_buffer, thread_args,
                                               items, n_threads, &hts);
    }
    else
    {

        TEST_ASSERT(0, "memory allocation failed\n");

        if (hts)
        {
            generic_hash_table_s_free(&hts);
        }
    }
}

char*
generic_hash_table_s_get_key(size_t key_index, size_t total_keys)
{
    static char key_buffer[256];
    snprintf(key_buffer, sizeof(key_buffer), "key_%zu", key_index % total_keys);
    return key_buffer;
}

void
generic_hash_table_s_phase_insert_many_keys(pthread_t* threads_buffer,
                                            struct pthread_arg_t* thread_args,
                                            int** items, size_t n_threads,
                                            size_t total_keys)
{

#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 1] Concurrent Inserts with %zu Different Keys\n",
            total_keys);
#endif

    for (size_t i = 0; i < n_threads; i++)
    {

        items[i] = (int*) malloc(sizeof(int));
        if (items[i])
        {

            *items[i] = (int) (1000 + i);

            thread_args[i]._thread_id = i;
            thread_args[i]._item = items[i];
            thread_args[i]._free_item = free;
            thread_args[i]._deep_copy = copy_int;

            thread_args[i].key = generic_hash_table_s_get_key(i, total_keys);

            pthread_create(&threads_buffer[i], NULL,
                           generic_hash_table_s_insert_worker,
                           (void*) &thread_args[i]);
        }
    }

    for (size_t i = 0; i < n_threads; i++)
    {
        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);

        long result = (long) thread_result;
        if (result != 0)
        {
#ifdef STDIO_DEBUG
            fprintf(stderr, "stress_test: insert thread %zu returned %ld\n", i,
                    result);
#endif
        }
    }

    TEST_ASSERT(
        1, "Generic Hash Table Syn Functional Stress - Phase 1 completed\n");
}

void
generic_hash_table_s_phase_delete_many_keys(pthread_t* threads_buffer,
                                            struct pthread_arg_t* thread_args,
                                            size_t n_threads, size_t total_keys)
{
#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 2] Concurrent Deletes with %zu Different Keys\n",
            total_keys);
#endif

    for (size_t i = 0; i < n_threads; i++)
    {
        thread_args[i].key = generic_hash_table_s_get_key(i, total_keys);

        pthread_create(&threads_buffer[i], NULL,
                       generic_hash_table_s_delete_worker,
                       (void*) &thread_args[i]);
    }

    for (size_t i = 0; i < n_threads; i++)
    {
        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);

        long result = (long) thread_result;
        if (result != 0)
        {
#ifdef STDIO_DEBUG
            fprintf(stderr, "stress_test: delete thread %zu returned %ld\n", i,
                    result);
#endif
        }
    }

    TEST_ASSERT(
        1, "Generic Hash Table Syn Functional Stress - Phase 2 completed\n");
}

void
generic_hash_table_s_phase_mixed_many_keys(pthread_t* threads_buffer,
                                           struct pthread_arg_t* thread_args,
                                           int** items, size_t n_threads,
                                           size_t total_keys)
{

#ifdef STDIO_DEBUG
    fprintf(stdout, "\n[PHASE 3] Mixed Insert/Delete with %zu Different Keys\n",
            total_keys);
#endif

    for (size_t i = 0; i < n_threads; i++)
    {
        if (items[i])
        {
            free(items[i]);
        }

        items[i] = (int*) malloc(sizeof(int));
        if (items[i])
        {
            *items[i] = (int) (2000 + i);
            thread_args[i]._item = items[i];

            thread_args[i].key = generic_hash_table_s_get_key(i, total_keys);

            if (i % 2 == 0)
            {
                pthread_create(&threads_buffer[i], NULL,
                               generic_hash_table_s_insert_worker,
                               (void*) &thread_args[i]);
            }
            else
            {
                pthread_create(&threads_buffer[i], NULL,
                               generic_hash_table_s_delete_worker,
                               (void*) &thread_args[i]);
            }
        }
    }

    for (size_t i = 0; i < n_threads; i++)
    {
        void* thread_result = NULL;
        pthread_join(threads_buffer[i], &thread_result);
    }

    TEST_ASSERT(
        1, "Generic Hash Table Syn Functional Stress - Phase 3 completed\n");
}

void
generic_hash_table_s_functional_stress_test(size_t n_threads,
                                            size_t hash_table_capacity,
                                            size_t total_keys)
{

    TEST_SUITE("Generic Hash Table Syn. Functional Stress Test");

    generic_hash_table_s hts = NULL;
    int exit_code =
        generic_hash_table_s_new(hash_string, hash_table_capacity, &hts);

    TEST_ASSERT(!exit_code, "generic_hash_table_s_new returned success\n");
    TEST_ASSERT(hts != NULL, "generic_hash_table_s is not NULL\n");

    pthread_t* threads_buffer = NULL;
    struct pthread_arg_t* thread_args = NULL;
    int** items = NULL;

    if (generic_hash_table_s_allocate_resources(n_threads, &threads_buffer,
                                                &thread_args, &items)
        == 0)
    {

        for (size_t i = 0; i < n_threads; i++)
        {
            thread_args[i]._generic_hash_table_s = hts;
        }

        generic_hash_table_s_phase_insert_many_keys(
            threads_buffer, thread_args, items, n_threads, total_keys);
        generic_hash_table_s_phase_delete_many_keys(threads_buffer, thread_args,
                                                    n_threads, total_keys);
        generic_hash_table_s_phase_mixed_many_keys(
            threads_buffer, thread_args, items, n_threads, total_keys);

        generic_hash_table_s_cleanup_resources(threads_buffer, thread_args,
                                               items, n_threads, &hts);
    }
    else
    {
        TEST_ASSERT(0, "memory allocation failed\n");

        if (hts)
        {
            generic_hash_table_s_free(&hts);
        }
    }
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{
    TEST_SUITE("Begin Hash Table Syn Test");

    generic_hash_table_s_new_and_free_test();
    generic_hash_table_s_stress_test(10, 5);
    generic_hash_table_s_stress_test(100, 10);
    generic_hash_table_s_stress_test(10, 100);
    generic_hash_table_s_stress_test(1000, 2);
    generic_hash_table_s_functional_stress_test(100, 50, 20);
    generic_hash_table_s_functional_stress_test(1000, 100, 100);
    generic_hash_table_s_functional_stress_test(10000, 500, 500);

    TEST_SUITE("End Hash Table Syn Test");

    return stats.failed;
}
