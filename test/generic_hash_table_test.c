#include "generic_hash_table.h"
#include "test_utils.h"
#include <stdlib.h>

void
generic_hash_table_new_and_free_test(void)
{
    TEST_SUITE("Generic Hash Table New and Free Test");

    size_t capacity = 3;
    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, capacity, &ht);

    TEST_ASSERT(ht != NULL, "Hash Table instance created successfully");
    TEST_ASSERT(result == 0, "Hash Table creation returns success");

    size_t check_capacity = 0;
    generic_hash_table_get_capacity(ht, &check_capacity);
    TEST_ASSERT(check_capacity == capacity,
                "Hash Table instance has the correct initial capacity");

    size_t (*hash_function)(void*) = NULL;
    generic_hash_table_get_hash_function(ht, &hash_function);
    TEST_ASSERT(hash_function == hash_string,
                "Hash Table instance has the correct hash function");

    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
generic_hash_table_insert_with_free_item_function_as_NULL_test(void)
{

    TEST_SUITE("Generic Hash Table Insert with Free Item Funtion as NULL Test");

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    char* key_0 = "key 0";
    int item_0 = 3;
    char* key_1 = "key 1";
    int item_1 = 10;

    result = generic_hash_table_insert(ht, key_0, &item_0, NULL, NULL);
    TEST_ASSERT(result == 0, "Hash Table has inserted the pair (\"key 0\", 3)");
    result = generic_hash_table_insert(ht, key_1, &item_1, NULL, NULL);
    TEST_ASSERT(result == 0,
                "Hash Table has inserted the pair (\"key 1\", 10)");

    int* check_item_0 = NULL;
    int* check_item_1 = NULL;

    result = generic_hash_table_get(ht, key_0, (void**) &check_item_0);
    TEST_ASSERT(
        result == 0,
        "Hash Table completed successfully the get \"key 0\" operation");
    TEST_ASSERT(*check_item_0 == item_0,
                "The insertion of the pair (\"key 0\", 3) has been validated");

    result = generic_hash_table_get(ht, key_1, (void**) &check_item_1);
    TEST_ASSERT(
        result == 0,
        "Hash Table completed successfully the get \"key 1\" operation");
    TEST_ASSERT(*check_item_1 == item_1,
                "The insertion of the pair (\"key 1\", 10) has been validated");

    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
free_int(void* i)
{

    if (!i)
    {
        return;
    }

    free(i);
}

int
copy_int(void *i, void **out_i)
{

    if (!i || !out_i)
    {
        return 1;
    }

    int* new_i = malloc(sizeof(int));
    if (!new_i)
    {
        return -1;
    }

    *new_i = *(int*) i;
    *out_i = new_i;

    return 0;
}

void
generic_hash_table_insert_get_free_flow_test(void)
{

    TEST_SUITE("Generic Hash Table Insert Basic Flow Test");

    // New

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    // Insert

    char* key_0 = "key 0";
    int* item_0 = malloc(sizeof(int));
    *item_0 = 3;

    char* key_1 = "key 1";
    int* item_1 = malloc(sizeof(int));
    *item_1 = 13;

    result = generic_hash_table_insert(ht, key_0, item_0, free_int, copy_int);
    TEST_ASSERT(result == 0, "Hash Table has inserted the pair (\"key 0\", 3)");
    result = generic_hash_table_insert(ht, key_1, item_1, free_int, copy_int);
    TEST_ASSERT(result == 0,
                "Hash Table has inserted the pair (\"key 1\", 10)");

    // Get

    int* check_item_0 = NULL;
    int* check_item_1 = NULL;

    result = generic_hash_table_get(ht, key_0, (void**) &check_item_0);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 0\" operation");
    TEST_ASSERT(check_item_0 != item_0,
                "Item: (\"key 0\", 3) pointer has been validated");
    TEST_ASSERT(*check_item_0 == *item_0,
                "Item: (\"key 0\", 3) value has been validated");

    result = generic_hash_table_get(ht, key_1, (void**) &check_item_1);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 1\" operation");
    TEST_ASSERT(check_item_1 != item_1,
                "Item: (\"key 1\", 13) pointer has been validated");
    TEST_ASSERT(*check_item_1 == *item_1,
                "Item: (\"key 1\", 13) value has been validated");

    // Free

    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
generic_hash_table_insert_tricky_flow_test(void)
{

    TEST_SUITE("Generic Hash Table Insert Basic Flow Test");

    // New

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    // Insert

    char* key_0 = "key 0";
    int* item_0 = malloc(sizeof(int));
    *item_0 = 3;

    char* key_1 = "key 1";
    int* item_1 = malloc(sizeof(int));
    *item_1 = 13;

    result = generic_hash_table_insert(ht, key_0, item_0, free_int, copy_int);
    TEST_ASSERT(result == 0, "Hash Table has inserted the pair (\"key 0\", 3)");
    result = generic_hash_table_insert(ht, key_1, item_1, free_int, copy_int);
    TEST_ASSERT(result == 0,
                "Hash Table has inserted the pair (\"key 1\", 10)");

    // Get

    int* check_item_0 = NULL;
    int* check_item_1 = NULL;

    result = generic_hash_table_get(ht, key_0, (void**) &check_item_0);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 0\" operation");
    TEST_ASSERT(check_item_0 != item_0,
                "Item: (\"key 0\", 3) pointer has been validated");
    TEST_ASSERT(*check_item_0 == *item_0,
                "Item: (\"key 0\", 3) value has been validated");

    result = generic_hash_table_get(ht, key_1, (void**) &check_item_1);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 1\" operation");
    TEST_ASSERT(check_item_1 != item_1,
                "Item: (\"key 1\", 13) pointer has been validated");
    TEST_ASSERT(*check_item_1 == *item_1,
                "Item: (\"key 1\", 13) value has been validated");

    // Free

    // Force manual free of an item
    free(item_0);
    item_0 = NULL;
    free(item_1);
    item_1 = NULL;

    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

int
main(int argc __attribute__((unused)), char** argv __attribute((unused)))
{

    printf("\n");
    printf("*****************************************\n");
    printf("Begin Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    generic_hash_table_new_and_free_test();
    generic_hash_table_insert_with_free_item_function_as_NULL_test();
    generic_hash_table_insert_get_free_flow_test();
    generic_hash_table_insert_tricky_flow_test();

    printf("\n");
    printf("*****************************************\n");
    printf("End Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}