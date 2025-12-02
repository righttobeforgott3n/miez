#include "generic_hash_table.h"
#include "test_utils.h"
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

    printf("\n");
    printf("*****************************************\n");
    printf("End Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}