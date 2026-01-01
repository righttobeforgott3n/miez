#include "generic_hash_table.h"
#include "test_utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

static size_t
int_hash(void* key)
{
    return (size_t) (*(int*) key);
}

static void
int_free(void* data)
{
    free(data);
}

static int
int_copy(void* src, void** dst)
{
    if (!src || !dst)
    {
        return 1;
    }

    int* copy = (int*) malloc(sizeof(int));
    if (!copy)
    {
        return -1;
    }

    *copy = *(int*) src;
    *dst = copy;

    return 0;
}

static int
int_compare(void* a, void* b)
{
    return *(int*) a - *(int*) b;
}

int
generic_hash_table_new_valid_test()
{
    TEST_SUITE("Generic Hash Table New Valid Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(10, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);

    TEST_ASSERT(result == 0, "new should return 0 on success");
    TEST_ASSERT(table != NULL, "new should set out_self");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_new_null_out_self_test()
{
    TEST_SUITE("Generic Hash Table New Null Out Self Test");

    int result = generic_hash_table_new(10, int_hash, int_free, int_copy,
                                        int_free, int_copy, int_compare, NULL);

    TEST_ASSERT(result == 1, "new should return 1 when out_self is NULL");

    return 0;
}

int
generic_hash_table_new_zero_capacity_test()
{
    TEST_SUITE("Generic Hash Table New Zero Capacity Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(0, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);

    TEST_ASSERT(result == 1, "new should return 1 when capacity is 0");
    TEST_ASSERT(table == NULL, "new should not set out_self on failure");

    return 0;
}

int
generic_hash_table_new_null_functions_test()
{
    TEST_SUITE("Generic Hash Table New Null Functions Test");

    generic_hash_table table = NULL;

    int result = generic_hash_table_new(10, NULL, int_free, int_copy, int_free,
                                        int_copy, int_compare, &table);
    TEST_ASSERT(result == 1, "new should return 1 when hash_function is NULL");

    result = generic_hash_table_new(10, int_hash, NULL, int_copy, int_free,
                                    int_copy, int_compare, &table);
    TEST_ASSERT(result == 1,
                "new should return 1 when free_value_function is NULL");

    result = generic_hash_table_new(10, int_hash, int_free, NULL, int_free,
                                    int_copy, int_compare, &table);
    TEST_ASSERT(result == 1,
                "new should return 1 when copy_value_function is NULL");

    result = generic_hash_table_new(10, int_hash, int_free, int_copy, NULL,
                                    int_copy, int_compare, &table);
    TEST_ASSERT(result == 1,
                "new should return 1 when free_key_function is NULL");

    result = generic_hash_table_new(10, int_hash, int_free, int_copy, int_free,
                                    NULL, int_compare, &table);
    TEST_ASSERT(result == 1,
                "new should return 1 when copy_key_function is NULL");

    result = generic_hash_table_new(10, int_hash, int_free, int_copy, int_free,
                                    int_copy, NULL, &table);
    TEST_ASSERT(result == 1,
                "new should return 1 when compare_key_function is NULL");

    return 0;
}

int
generic_hash_table_free_null_test()
{
    TEST_SUITE("Generic Hash Table Free Null Test");

    int result = generic_hash_table_free(NULL);

    TEST_ASSERT(result == 1, "free should return 1 when self is NULL");

    return 0;
}

int
generic_hash_table_is_empty_on_new_test()
{
    TEST_SUITE("Generic Hash Table Is Empty On New Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int result = generic_hash_table_is_empty(table);

    TEST_ASSERT(result == 1, "is_empty should return 1 for new table");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_is_empty_null_test()
{
    TEST_SUITE("Generic Hash Table Is Empty Null Test");

    int result = generic_hash_table_is_empty(NULL);

    TEST_ASSERT(result == -1, "is_empty should return -1 when self is NULL");

    return 0;
}

int
generic_hash_table_get_capacity_test()
{
    TEST_SUITE("Generic Hash Table Get Capacity Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    size_t capacity = 0;
    int result = generic_hash_table_get_capacity(table, &capacity);

    TEST_ASSERT(result == 0, "get_capacity should return 0 on success");
    TEST_ASSERT(capacity == 10, "get_capacity should return correct capacity");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_get_size_test()
{
    TEST_SUITE("Generic Hash Table Get Size Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    size_t size = 999;
    int result = generic_hash_table_get_size(table, &size);

    TEST_ASSERT(result == 0, "get_size should return 0 on success");
    TEST_ASSERT(size == 0, "get_size should return 0 for new table");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_insert_single_test()
{
    TEST_SUITE("Generic Hash Table Insert Single Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    int value = 100;
    int result = generic_hash_table_insert(table, &key, &value);

    TEST_ASSERT(result == 0, "insert should return 0 on success");

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == 1, "size should be 1 after insert");

    TEST_ASSERT(generic_hash_table_is_empty(table) == 0,
                "is_empty should return 0 after insert");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_insert_null_self_test()
{
    TEST_SUITE("Generic Hash Table Insert Null Self Test");

    int key = 42;
    int value = 100;
    int result = generic_hash_table_insert(NULL, &key, &value);

    TEST_ASSERT(result == 1, "insert should return 1 when self is NULL");

    return 0;
}

int
generic_hash_table_insert_null_key_test()
{
    TEST_SUITE("Generic Hash Table Insert Null Key Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int value = 100;
    int result = generic_hash_table_insert(table, NULL, &value);

    TEST_ASSERT(result == 1, "insert should return 1 when key is NULL");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_insert_multiple_test()
{
    TEST_SUITE("Generic Hash Table Insert Multiple Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    size_t i = 0;
    while (i < 5)
    {
        int key = i;
        int value = i * 10;
        generic_hash_table_insert(table, &key, &value);
        i++;
    }

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == 5, "size should be 5 after 5 inserts");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_get_existing_test()
{
    TEST_SUITE("Generic Hash Table Get Existing Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    int value = 100;
    generic_hash_table_insert(table, &key, &value);

    void* retrieved = NULL;
    int result = generic_hash_table_get(table, &key, &retrieved);

    TEST_ASSERT(result == 0, "get should return 0 when key exists");
    TEST_ASSERT(retrieved != NULL, "get should set out_value");
    TEST_ASSERT(*(int*) retrieved == 100, "get should return correct value");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_get_nonexistent_test()
{
    TEST_SUITE("Generic Hash Table Get Nonexistent Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    void* retrieved = NULL;
    int result = generic_hash_table_get(table, &key, &retrieved);

    TEST_ASSERT(result == 1, "get should return 1 when key does not exist");
    TEST_ASSERT(retrieved == NULL, "get should set out_value to NULL");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_get_null_params_test()
{
    TEST_SUITE("Generic Hash Table Get Null Params Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    void* retrieved = NULL;

    int result = generic_hash_table_get(NULL, &key, &retrieved);
    TEST_ASSERT(result == 1, "get should return 1 when self is NULL");

    result = generic_hash_table_get(table, NULL, &retrieved);
    TEST_ASSERT(result == 1, "get should return 1 when key is NULL");

    result = generic_hash_table_get(table, &key, NULL);
    TEST_ASSERT(result == 1, "get should return 1 when out_value is NULL");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_contains_existing_test()
{
    TEST_SUITE("Generic Hash Table Contains Existing Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    int value = 100;
    generic_hash_table_insert(table, &key, &value);

    int result = generic_hash_table_contains(table, &key);

    TEST_ASSERT(result == 0, "contains should return 0 when key exists");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_contains_nonexistent_test()
{
    TEST_SUITE("Generic Hash Table Contains Nonexistent Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    int result = generic_hash_table_contains(table, &key);

    TEST_ASSERT(result == 1,
                "contains should return 1 when key does not exist");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_delete_existing_test()
{
    TEST_SUITE("Generic Hash Table Delete Existing Test");

    generic_hash_table table = NULL;
    int exit_code =
        generic_hash_table_new(10, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(!exit_code, "new should return 0");

    int key = 42;
    int value = 100;
    exit_code = generic_hash_table_insert(table, &key, &value);
    TEST_ASSERT(!exit_code, "insert should return 0");

    exit_code = generic_hash_table_delete(table, &key);
    TEST_ASSERT(!exit_code, "delete should return 0 on success");

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == 0, "size should be 0 after delete");

    TEST_ASSERT(generic_hash_table_contains(table, &key) == 1,
                "contains should return 1 after delete");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_delete_nonexistent_test()
{
    TEST_SUITE("Generic Hash Table Delete Nonexistent Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;
    int result = generic_hash_table_delete(table, &key);

    TEST_ASSERT(result == 1, "delete should return 1 when key does not exist");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_delete_null_params_test()
{
    TEST_SUITE("Generic Hash Table Delete Null Params Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key = 42;

    int result = generic_hash_table_delete(NULL, &key);
    TEST_ASSERT(result == 1, "delete should return 1 when self is NULL");

    result = generic_hash_table_delete(table, NULL);
    TEST_ASSERT(result == 1, "delete should return 1 when key is NULL");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_collision_handling_test()
{
    TEST_SUITE("Generic Hash Table Collision Handling Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(2, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key1 = 0;
    int value1 = 100;
    int key2 = 2;
    int value2 = 200;

    generic_hash_table_insert(table, &key1, &value1);
    generic_hash_table_insert(table, &key2, &value2);

    void* retrieved = NULL;

    generic_hash_table_get(table, &key1, &retrieved);
    TEST_ASSERT(*(int*) retrieved == 100,
                "get should return correct value for first key in collision");

    generic_hash_table_get(table, &key2, &retrieved);
    TEST_ASSERT(*(int*) retrieved == 200,
                "get should return correct value for second key in collision");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_get_functions_test()
{
    TEST_SUITE("Generic Hash Table Get Functions Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(10, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    size_t (*hash_func)(void*) = NULL;
    generic_hash_table_get_hash_function(table, &hash_func);
    TEST_ASSERT(hash_func == int_hash,
                "get_hash_function should return correct function");

    void (*free_value_func)(void*) = NULL;
    generic_hash_table_get_free_value_function(table, &free_value_func);
    TEST_ASSERT(free_value_func == int_free,
                "get_free_value_function should return correct function");

    int (*copy_value_func)(void*, void**) = NULL;
    generic_hash_table_get_copy_value_function(table, &copy_value_func);
    TEST_ASSERT(copy_value_func == int_copy,
                "get_copy_value_function should return correct function");

    void (*free_key_func)(void*) = NULL;
    generic_hash_table_get_free_key_function(table, &free_key_func);
    TEST_ASSERT(free_key_func == int_free,
                "get_free_key_function should return correct function");

    int (*copy_key_func)(void*, void**) = NULL;
    generic_hash_table_get_copy_key_function(table, &copy_key_func);
    TEST_ASSERT(copy_key_func == int_copy,
                "get_copy_key_function should return correct function");

    int (*compare_key_func)(void*, void*) = NULL;
    generic_hash_table_get_compare_key_function(table, &compare_key_func);
    TEST_ASSERT(compare_key_func == int_compare,
                "get_compare_key_function should return correct function");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_stress_test()
{
    TEST_SUITE("Generic Hash Table Stress Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(100, int_hash, int_free, int_copy, int_free,
                           int_copy, int_compare, &table);

    size_t n = 1000;
    size_t i = 0;
    while (i < n)
    {
        int key = i;
        int value = i * 10;
        int result = generic_hash_table_insert(table, &key, &value);
        if (result)
        {
            TEST_ASSERT(0, "insert failed during stress test");
            generic_hash_table_free(table);
            return 1;
        }
        i++;
    }

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == n, "size should match number of inserts");

    i = 0;
    while (i < n)
    {
        int key = i;
        void* retrieved = NULL;
        int result = generic_hash_table_get(table, &key, &retrieved);
        if (result || *(int*) retrieved != (int) (i * 10))
        {
            TEST_ASSERT(0, "get failed during stress test");
            generic_hash_table_free(table);
            return 1;
        }
        i++;
    }
    TEST_ASSERT(1, "all get operations succeeded");

    i = 0;
    while (i < n / 2)
    {
        int key = i;
        int result = generic_hash_table_delete(table, &key);
        if (result)
        {
            TEST_ASSERT(0, "delete failed during stress test");
            generic_hash_table_free(table);
            return 1;
        }
        i++;
    }

    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == n / 2, "size should be half after deletions");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_delete_middle_collision_test()
{
    TEST_SUITE("Generic Hash Table Delete Middle Collision Test");

    generic_hash_table table = NULL;
    generic_hash_table_new(1, int_hash, int_free, int_copy, int_free, int_copy,
                           int_compare, &table);

    int key1 = 1;
    int value1 = 100;
    int key2 = 2;
    int value2 = 200;
    int key3 = 3;
    int value3 = 300;

    generic_hash_table_insert(table, &key1, &value1);
    generic_hash_table_insert(table, &key2, &value2);
    generic_hash_table_insert(table, &key3, &value3);

    int result = generic_hash_table_delete(table, &key2);
    TEST_ASSERT(result == 0, "delete middle element should succeed");

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == 2, "size should be 2 after delete");

    void* retrieved = NULL;
    result = generic_hash_table_get(table, &key1, &retrieved);
    TEST_ASSERT(result == 0 && *(int*) retrieved == 100,
                "key1 should still exist");

    result = generic_hash_table_get(table, &key2, &retrieved);
    TEST_ASSERT(result == 1, "key2 should not exist");

    result = generic_hash_table_get(table, &key3, &retrieved);
    TEST_ASSERT(result == 0 && *(int*) retrieved == 300,
                "key3 should still exist");

    generic_hash_table_free(table);

    return 0;
}

typedef struct
{
    generic_hash_table table;
    int thread_id;
    int num_operations;
    int start_key;
    atomic_int* success_count;
    atomic_int* failure_count;
} concurrent_thread_args_t;

typedef struct
{
    generic_hash_table table;
    int thread_id;
    int num_operations;
    int key_range;
    atomic_int* insert_count;
    atomic_int* get_count;
    atomic_int* delete_count;
} mixed_ops_thread_args_t;

void*
concurrent_insert_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_success = 0;
    int local_failure = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = args->start_key + i;
        int value = args->thread_id * 100000 + i;

        int result = generic_hash_table_insert(args->table, &key, &value);
        if (result == 0)
        {
            local_success++;
        }
        else
        {
            local_failure++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_success);
    atomic_fetch_add(args->failure_count, local_failure);

    return NULL;
}

int
generic_hash_table_concurrent_insert_different_keys_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Insert Different Keys Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(64, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 8;
    const int ops_per_thread = 1000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int success_count = 0;
    atomic_int failure_count = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = i * ops_per_thread;
        args[i].success_count = &success_count;
        args[i].failure_count = &failure_count;
        pthread_create(&threads[i], NULL, concurrent_insert_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t size = 0;
    generic_hash_table_get_size(table, &size);

    TEST_ASSERT(size == (size_t) (num_threads * ops_per_thread),
                "All inserts should succeed with different keys");
    TEST_ASSERT(atomic_load(&failure_count) == 0,
                "No insert failures expected");

    generic_hash_table_free(table);

    return 0;
}

int
generic_hash_table_concurrent_insert_same_bucket_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Insert Same Bucket Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(1, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created with single bucket");

    const int num_threads = 8;
    const int ops_per_thread = 500;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int success_count = 0;
    atomic_int failure_count = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = i * ops_per_thread;
        args[i].success_count = &success_count;
        args[i].failure_count = &failure_count;
        pthread_create(&threads[i], NULL, concurrent_insert_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t size = 0;
    generic_hash_table_get_size(table, &size);

    TEST_ASSERT(size == (size_t) (num_threads * ops_per_thread),
                "All inserts should succeed even in same bucket");

    generic_hash_table_free(table);

    return 0;
}

void*
concurrent_get_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_success = 0;
    int local_failure = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = i % 1000;
        void* value = NULL;

        int result = generic_hash_table_get(args->table, &key, &value);
        if (result == 0 && value != NULL)
        {
            local_success++;
        }
        else
        {
            local_failure++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_success);
    atomic_fetch_add(args->failure_count, local_failure);

    return NULL;
}

int
generic_hash_table_concurrent_get_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Get Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(64, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    int i = 0;
    while (i < 1000)
    {
        int key = i;
        int value = i * 10;
        generic_hash_table_insert(table, &key, &value);
        i++;
    }

    size_t initial_size = 0;
    generic_hash_table_get_size(table, &initial_size);
    TEST_ASSERT(initial_size == 1000, "Table pre-populated with 1000 items");

    const int num_threads = 16;
    const int ops_per_thread = 5000;
    pthread_t threads[16];
    concurrent_thread_args_t args[16];
    atomic_int success_count = 0;
    atomic_int failure_count = 0;

    i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = 0;
        args[i].success_count = &success_count;
        args[i].failure_count = &failure_count;
        pthread_create(&threads[i], NULL, concurrent_get_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(atomic_load(&success_count) == num_threads * ops_per_thread,
                "All gets should succeed for existing keys");

    size_t final_size = 0;
    generic_hash_table_get_size(table, &final_size);
    TEST_ASSERT(final_size == initial_size,
                "Table size unchanged after concurrent gets");

    generic_hash_table_free(table);

    return 0;
}

void*
concurrent_delete_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_success = 0;
    int local_failure = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = args->start_key + i;

        int result = generic_hash_table_delete(args->table, &key);
        if (result == 0)
        {
            local_success++;
        }
        else
        {
            local_failure++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_success);
    atomic_fetch_add(args->failure_count, local_failure);

    return NULL;
}

int
generic_hash_table_concurrent_delete_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Delete Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(64, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int total_items = 8000;
    int i = 0;
    while (i < total_items)
    {
        int key = i;
        int value = i * 10;
        generic_hash_table_insert(table, &key, &value);
        i++;
    }

    size_t initial_size = 0;
    generic_hash_table_get_size(table, &initial_size);
    TEST_ASSERT(initial_size == (size_t) total_items, "Table pre-populated");

    const int num_threads = 8;
    const int ops_per_thread = 1000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int success_count = 0;
    atomic_int failure_count = 0;

    i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = i * ops_per_thread;
        args[i].success_count = &success_count;
        args[i].failure_count = &failure_count;
        pthread_create(&threads[i], NULL, concurrent_delete_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    size_t final_size = 0;
    generic_hash_table_get_size(table, &final_size);

    TEST_ASSERT(atomic_load(&success_count) == num_threads * ops_per_thread,
                "All deletes should succeed for existing keys");
    TEST_ASSERT(final_size == initial_size - (num_threads * ops_per_thread),
                "Size reduced by number of deletes");

    generic_hash_table_free(table);

    return 0;
}

void*
mixed_operations_thread(void* arg)
{
    mixed_ops_thread_args_t* args = (mixed_ops_thread_args_t*) arg;
    int local_inserts = 0;
    int local_gets = 0;
    int local_deletes = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int op = i % 3;
        int key = (args->thread_id * 10000 + i) % args->key_range;
        int value = i;

        if (op == 0)
        {
            if (generic_hash_table_insert(args->table, &key, &value) == 0)
            {
                local_inserts++;
            }
        }
        else if (op == 1)
        {
            void* retrieved = NULL;
            if (generic_hash_table_get(args->table, &key, &retrieved) == 0)
            {
                local_gets++;
            }
        }
        else
        {
            if (generic_hash_table_delete(args->table, &key) == 0)
            {
                local_deletes++;
            }
        }
        i++;
    }

    atomic_fetch_add(args->insert_count, local_inserts);
    atomic_fetch_add(args->get_count, local_gets);
    atomic_fetch_add(args->delete_count, local_deletes);

    return NULL;
}

int
generic_hash_table_concurrent_mixed_operations_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Mixed Operations Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(32, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    int i = 0;
    while (i < 500)
    {
        int key = i;
        int value = i;
        generic_hash_table_insert(table, &key, &value);
        i++;
    }

    const int num_threads = 12;
    const int ops_per_thread = 3000;
    pthread_t threads[12];
    mixed_ops_thread_args_t args[12];
    atomic_int insert_count = 0;
    atomic_int get_count = 0;
    atomic_int delete_count = 0;

    i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].key_range = 1000;
        args[i].insert_count = &insert_count;
        args[i].get_count = &get_count;
        args[i].delete_count = &delete_count;
        pthread_create(&threads[i], NULL, mixed_operations_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "No crashes during mixed concurrent operations");

    generic_hash_table_free(table);

    return 0;
}

void*
stress_worker_thread(void* arg)
{
    mixed_ops_thread_args_t* args = (mixed_ops_thread_args_t*) arg;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = (args->thread_id * 100000 + i) % args->key_range;
        int value = i;

        int op = (i * 7 + args->thread_id) % 5;

        if (op < 2)
        {
            generic_hash_table_insert(args->table, &key, &value);
            atomic_fetch_add(args->insert_count, 1);
        }
        else if (op < 4)
        {
            void* retrieved = NULL;
            generic_hash_table_get(args->table, &key, &retrieved);
            atomic_fetch_add(args->get_count, 1);
        }
        else
        {
            generic_hash_table_delete(args->table, &key);
            atomic_fetch_add(args->delete_count, 1);
        }
        i++;
    }

    return NULL;
}

int
generic_hash_table_high_load_stress_test()
{
    TEST_SUITE("Generic Hash Table High Load Stress Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(128, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 16;
    const int ops_per_thread = 10000;
    pthread_t threads[16];
    mixed_ops_thread_args_t args[16];
    atomic_int insert_count = 0;
    atomic_int get_count = 0;
    atomic_int delete_count = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].key_range = 5000;
        args[i].insert_count = &insert_count;
        args[i].get_count = &get_count;
        args[i].delete_count = &delete_count;
        pthread_create(&threads[i], NULL, stress_worker_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    int total_ops = atomic_load(&insert_count) + atomic_load(&get_count)
                    + atomic_load(&delete_count);

    TEST_ASSERT(total_ops == num_threads * ops_per_thread,
                "All operations completed");
    TEST_ASSERT(1, "No crashes or deadlocks during high load");

    generic_hash_table_free(table);

    return 0;
}

void*
contention_insert_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_success = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = i % 100;
        int value = args->thread_id * 1000 + i;

        if (generic_hash_table_insert(args->table, &key, &value) == 0)
        {
            local_success++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_success);

    return NULL;
}

int
generic_hash_table_contention_same_keys_test()
{
    TEST_SUITE("Generic Hash Table Contention Same Keys Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(16, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 8;
    const int ops_per_thread = 1000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int success_count = 0;
    atomic_int failure_count = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = 0;
        args[i].success_count = &success_count;
        args[i].failure_count = &failure_count;
        pthread_create(&threads[i], NULL, contention_insert_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(atomic_load(&success_count) == num_threads * ops_per_thread,
                "All inserts should complete");

    generic_hash_table_free(table);

    return 0;
}

typedef struct
{
    generic_hash_table table;
    int start_key;
    int num_keys;
    atomic_int* verified_count;
    atomic_int* error_count;
} integrity_thread_args_t;

void*
integrity_insert_thread(void* arg)
{
    integrity_thread_args_t* args = (integrity_thread_args_t*) arg;

    int i = 0;
    while (i < args->num_keys)
    {
        int key = args->start_key + i;
        int value = key * 10;
        generic_hash_table_insert(args->table, &key, &value);
        i++;
    }

    return NULL;
}

void*
integrity_verify_thread(void* arg)
{
    integrity_thread_args_t* args = (integrity_thread_args_t*) arg;
    int local_verified = 0;
    int local_errors = 0;

    int i = 0;
    while (i < args->num_keys)
    {
        int key = args->start_key + i;
        void* value = NULL;

        if (generic_hash_table_get(args->table, &key, &value) == 0
            && value != NULL)
        {
            int expected = key * 10;
            if (*(int*) value == expected)
            {
                local_verified++;
            }
            else
            {
                local_errors++;
            }
        }
        i++;
    }

    atomic_fetch_add(args->verified_count, local_verified);
    atomic_fetch_add(args->error_count, local_errors);

    return NULL;
}

int
generic_hash_table_data_integrity_test()
{
    TEST_SUITE("Generic Hash Table Data Integrity Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(64, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 4;
    const int keys_per_thread = 1000;
    pthread_t insert_threads[4];
    integrity_thread_args_t insert_args[4];

    int i = 0;
    while (i < num_threads)
    {
        insert_args[i].table = table;
        insert_args[i].start_key = i * keys_per_thread;
        insert_args[i].num_keys = keys_per_thread;
        insert_args[i].verified_count = NULL;
        insert_args[i].error_count = NULL;
        pthread_create(&insert_threads[i], NULL, integrity_insert_thread,
                       &insert_args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(insert_threads[i], NULL);
        i++;
    }

    size_t size = 0;
    generic_hash_table_get_size(table, &size);
    TEST_ASSERT(size == (size_t) (num_threads * keys_per_thread),
                "All items inserted");

    pthread_t verify_threads[4];
    integrity_thread_args_t verify_args[4];
    atomic_int verified_count = 0;
    atomic_int error_count = 0;

    i = 0;
    while (i < num_threads)
    {
        verify_args[i].table = table;
        verify_args[i].start_key = i * keys_per_thread;
        verify_args[i].num_keys = keys_per_thread;
        verify_args[i].verified_count = &verified_count;
        verify_args[i].error_count = &error_count;
        pthread_create(&verify_threads[i], NULL, integrity_verify_thread,
                       &verify_args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(verify_threads[i], NULL);
        i++;
    }

    TEST_ASSERT(atomic_load(&verified_count) == num_threads * keys_per_thread,
                "All items verified correctly");
    TEST_ASSERT(atomic_load(&error_count) == 0, "No data corruption");

    generic_hash_table_free(table);

    return 0;
}

void*
concurrent_contains_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_found = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = i % 500;
        if (generic_hash_table_contains(args->table, &key) == 0)
        {
            local_found++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_found);

    return NULL;
}

int
generic_hash_table_concurrent_contains_test()
{
    TEST_SUITE("Generic Hash Table Concurrent Contains Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(32, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    int i = 0;
    while (i < 500)
    {
        int key = i;
        int value = i;
        generic_hash_table_insert(table, &key, &value);
        i++;
    }

    const int num_threads = 8;
    const int ops_per_thread = 5000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int found_count = 0;
    atomic_int dummy = 0;

    i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = 0;
        args[i].success_count = &found_count;
        args[i].failure_count = &dummy;
        pthread_create(&threads[i], NULL, concurrent_contains_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(atomic_load(&found_count) == num_threads * ops_per_thread,
                "All contains should succeed for existing keys");

    generic_hash_table_free(table);

    return 0;
}

void*
rapid_insert_delete_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;
    int local_success = 0;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = args->thread_id;
        int value = i;

        if (generic_hash_table_insert(args->table, &key, &value) == 0)
        {
            local_success++;
        }
        if (generic_hash_table_delete(args->table, &key) == 0)
        {
            local_success++;
        }
        i++;
    }

    atomic_fetch_add(args->success_count, local_success);

    return NULL;
}

int
generic_hash_table_rapid_insert_delete_test()
{
    TEST_SUITE("Generic Hash Table Rapid Insert/Delete Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(16, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 8;
    const int ops_per_thread = 5000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int success_count = 0;
    atomic_int dummy = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = i;
        args[i].success_count = &success_count;
        args[i].failure_count = &dummy;
        pthread_create(&threads[i], NULL, rapid_insert_delete_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "No crashes during rapid insert/delete");

    generic_hash_table_free(table);

    return 0;
}

void*
empty_table_access_thread(void* arg)
{
    concurrent_thread_args_t* args = (concurrent_thread_args_t*) arg;

    int i = 0;
    while (i < args->num_operations)
    {
        int key = i;
        void* value = NULL;

        generic_hash_table_get(args->table, &key, &value);
        generic_hash_table_contains(args->table, &key);
        generic_hash_table_delete(args->table, &key);
        i++;
    }

    return NULL;
}

int
generic_hash_table_empty_table_concurrent_test()
{
    TEST_SUITE("Generic Hash Table Empty Table Concurrent Access Test");

    generic_hash_table table = NULL;
    int result =
        generic_hash_table_new(16, int_hash, int_free, int_copy, int_free,
                               int_copy, int_compare, &table);
    TEST_ASSERT(result == 0, "Table created");

    const int num_threads = 8;
    const int ops_per_thread = 1000;
    pthread_t threads[8];
    concurrent_thread_args_t args[8];
    atomic_int dummy1 = 0;
    atomic_int dummy2 = 0;

    int i = 0;
    while (i < num_threads)
    {
        args[i].table = table;
        args[i].thread_id = i;
        args[i].num_operations = ops_per_thread;
        args[i].start_key = 0;
        args[i].success_count = &dummy1;
        args[i].failure_count = &dummy2;
        pthread_create(&threads[i], NULL, empty_table_access_thread, &args[i]);
        i++;
    }

    i = 0;
    while (i < num_threads)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    TEST_ASSERT(1, "No crashes on empty table concurrent access");

    int is_empty = generic_hash_table_is_empty(table);
    TEST_ASSERT(is_empty == 1, "Table is still empty");

    generic_hash_table_free(table);

    return 0;
}

struct thread_increment_arg_t
{
    generic_hash_table ht;
    int* key;
};

void
increment(int* v)
{
    printf("Value before: %d\n", *v);
    (*v)++;
    printf("Value after: %d\n", *v);
}

void*
value_increment_procedure(void* t_arg)
{

    struct thread_increment_arg_t* tia = (struct thread_increment_arg_t*) t_arg;
    generic_hash_table ht = tia->ht;
    int* key = tia->key;
    printf("Key: %d\n", *key);

    int exit_code = generic_hash_table_apply_on(ht, (void*) key,
                                                (void (*)(void*)) increment);
    if (exit_code)
    {
        fprintf(stderr, "Error during apply - exit code: %d\n", exit_code);
        return NULL;
    }

    return NULL;
}

void
generic_hash_table_atomic_apply_test(void)
{

    TEST_SUITE("Generic Hash Table Atomic Apply Test");

    generic_hash_table ht = NULL;
    int exit_code = generic_hash_table_new(
        16, int_hash, int_free, int_copy, int_free, int_copy, int_compare, &ht);
    TEST_ASSERT(exit_code == 0, "Hash Table Created");

    int key = 1;
    int value = 0;
    exit_code = generic_hash_table_insert(ht, (void*) &key, (void*) &value);
    TEST_ASSERT(exit_code == 0, "Hash Table Insert");

    int* value_got = NULL;
    exit_code = generic_hash_table_get(ht, (void*) &key, (void**) &value_got);
    TEST_ASSERT(exit_code == 0, "Hash Table Get");
    TEST_ASSERT(*value_got == 0, "Hash Table Value Got");

    struct thread_increment_arg_t arg;
    arg.ht = ht;
    arg.key = &key;

    pthread_t threads[100];
    int i = 0;
    while (i < 100)
    {
        pthread_create(&threads[i], NULL, value_increment_procedure, &arg);
        i++;
    }

    i = 0;
    while (i < 100)
    {
        pthread_join(threads[i], NULL);
        i++;
    }

    int b = *value_got == 100;
    TEST_ASSERT(b, "Value has been incremented");
    if (!b)
    {
        fprintf(stderr, "Value: %d\n", *value_got);
    }

    exit_code = generic_hash_table_free(ht);
    TEST_ASSERT(exit_code == 0, "Hash Table Freed");
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    printf("\n");
    printf("*****************************************\n");
    printf("Begin Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    generic_hash_table_new_valid_test();
    generic_hash_table_new_null_out_self_test();
    generic_hash_table_new_zero_capacity_test();
    generic_hash_table_new_null_functions_test();

    generic_hash_table_free_null_test();

    generic_hash_table_is_empty_on_new_test();
    generic_hash_table_is_empty_null_test();

    generic_hash_table_get_capacity_test();
    generic_hash_table_get_size_test();

    generic_hash_table_insert_single_test();
    generic_hash_table_insert_null_self_test();
    generic_hash_table_insert_null_key_test();
    generic_hash_table_insert_multiple_test();

    generic_hash_table_get_existing_test();
    generic_hash_table_get_nonexistent_test();
    generic_hash_table_get_null_params_test();

    generic_hash_table_contains_existing_test();
    generic_hash_table_contains_nonexistent_test();

    generic_hash_table_delete_existing_test();
    generic_hash_table_delete_nonexistent_test();
    generic_hash_table_delete_null_params_test();

    generic_hash_table_collision_handling_test();
    generic_hash_table_delete_middle_collision_test();

    generic_hash_table_get_functions_test();

    generic_hash_table_stress_test();

    generic_hash_table_concurrent_insert_different_keys_test();
    generic_hash_table_concurrent_insert_same_bucket_test();
    generic_hash_table_concurrent_get_test();
    generic_hash_table_concurrent_delete_test();
    generic_hash_table_concurrent_mixed_operations_test();
    generic_hash_table_high_load_stress_test();
    generic_hash_table_contention_same_keys_test();
    generic_hash_table_data_integrity_test();
    generic_hash_table_concurrent_contains_test();
    generic_hash_table_rapid_insert_delete_test();
    generic_hash_table_empty_table_concurrent_test();

    generic_hash_table_atomic_apply_test();

    printf("\n");
    printf("*****************************************\n");
    printf("End Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}
