#include "generic_hash_table.h"
#include "test_utils.h"
#include <stdlib.h>
#include <string.h>

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
copy_int(void* i, void** out_i)
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

int
copy_string(void* str, void** out_str)
{

    if (!str || !out_str)
    {
        return 1;
    }

    char* original = (char*) str;
    size_t len = strlen(original);
    char* new_str = malloc(len + 1);
    if (!new_str)
    {
        return -1;
    }

    strcpy(new_str, original);
    *out_str = new_str;

    return 0;
}

void
free_string(void* str)
{

    if (!str)
    {
        return;
    }

    free(str);
}

void
generic_hash_table_insert_and_get_basic_test(void)
{

    TEST_SUITE("Generic Hash Table Insert and Get Basic Test");

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

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
                "Hash Table has inserted the pair (\"key 1\", 13)");

    int* check_item_0 = NULL;
    int* check_item_1 = NULL;
    int (*deep_copy)(void*, void**) = NULL;

    result =
        generic_hash_table_get(ht, key_0, (void**) &check_item_0, &deep_copy);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 0\" operation");
    TEST_ASSERT(check_item_0 != item_0,
                "Item: (\"key 0\", 3) pointer has been validated");
    TEST_ASSERT(*check_item_0 == *item_0,
                "Item: (\"key 0\", 3) value has been validated");
    TEST_ASSERT(deep_copy == copy_int,
                "Deep copy function has been validated for (\"key 0\", 3)\n");

    result =
        generic_hash_table_get(ht, key_1, (void**) &check_item_1, &deep_copy);
    TEST_ASSERT(result == 0, "Hash Table completed successfully the get on "
                             "key: \"key 1\" operation");
    TEST_ASSERT(check_item_1 != item_1,
                "Item: (\"key 1\", 13) pointer has been validated");
    TEST_ASSERT(*check_item_1 == *item_1,
                "Item: (\"key 1\", 13) value has been validated");
    TEST_ASSERT(deep_copy == copy_int,
                "Deep copy function has been validated for (\"key 1\", 13)\n");

    free(item_0);
    item_0 = NULL;
    free(item_1);
    item_1 = NULL;
    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
generic_hash_table_delete_test(void)
{

    TEST_SUITE("Generic Hash Table Delete Test");

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    char* key_0 = "key 0";
    int* item_0 = malloc(sizeof(int));
    *item_0 = 42;

    result = generic_hash_table_insert(ht, key_0, item_0, free_int, copy_int);
    TEST_ASSERT(result == 0,
                "Hash Table has inserted the pair (\"key 0\", 42)");

    int* check_item_0 = NULL;
    int (*deep_copy)(void*, void**) = NULL;

    result =
        generic_hash_table_get(ht, key_0, (void**) &check_item_0, &deep_copy);
    TEST_ASSERT(result == 0, "Hash Table get operation successful");
    TEST_ASSERT(check_item_0 != NULL, "Item retrieved is not NULL");
    TEST_ASSERT(*check_item_0 == 42, "Item value validated before delete");

    result = generic_hash_table_delete(ht, key_0);
    TEST_ASSERT(result == 0, "Hash Table delete operation successful");

    check_item_0 = NULL;
    deep_copy = NULL;
    result =
        generic_hash_table_get(ht, key_0, (void**) &check_item_0, &deep_copy);
    TEST_ASSERT(result == 0, "Hash Table get after delete successful");
    TEST_ASSERT(check_item_0 == NULL, "Item is NULL after deletion");

    free(item_0);
    item_0 = NULL;
    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
generic_hash_table_overwrite_test(void)
{

    TEST_SUITE("Generic Hash Table Overwrite Test");

    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, 10, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    char* key = "key";
    int* item_0 = malloc(sizeof(int));
    *item_0 = 100;

    result = generic_hash_table_insert(ht, key, item_0, free_int, copy_int);
    TEST_ASSERT(result == 0, "Hash Table has inserted the pair (\"key\", 100)");

    int* item_1 = malloc(sizeof(int));
    *item_1 = 200;

    result = generic_hash_table_insert(ht, key, item_1, free_int, copy_int);
    TEST_ASSERT(result == 0,
                "Hash Table has overwritten with pair (\"key\", 200)");

    int* check_item = NULL;
    int (*deep_copy)(void*, void**) = NULL;

    result = generic_hash_table_get(ht, key, (void**) &check_item, &deep_copy);
    TEST_ASSERT(result == 0, "Hash Table get operation successful");
    TEST_ASSERT(check_item != NULL, "Item retrieved is not NULL");
    TEST_ASSERT(*check_item == 200, "Item value is the new value (200)");

    free(item_0);
    item_0 = NULL;
    free(item_1);
    item_1 = NULL;
    result = generic_hash_table_free(ht);
    TEST_ASSERT(result == 0, "Hash Table have been freed successfully");
}

void
generic_hash_table_stress_test(void)
{

    TEST_SUITE("Generic Hash Table Stress Test");

    size_t capacity = 100;
    generic_hash_table ht = NULL;
    int result = generic_hash_table_new(hash_string, capacity, &ht);
    TEST_ASSERT(ht != NULL, "Hash Table instance not NULL");
    TEST_ASSERT(result == 0, "Hash Table creation ended succcessfully");

    size_t num_items = 1000;
    char** keys = malloc(sizeof(char*) * num_items);
    int** items = malloc(sizeof(int*) * num_items);

    TEST_ASSERT(keys != NULL, "Keys array allocated successfully");
    TEST_ASSERT(items != NULL, "Items array allocated successfully");

    size_t i = 0;
    while (i < num_items)
    {

        keys[i] = malloc(32);
        if (!keys[i])
        {
            TEST_ASSERT(0, "Key allocation failed");
            break;
        }

        snprintf(keys[i], 32, "key_%zu", i);

        items[i] = malloc(sizeof(int));
        if (!items[i])
        {
            TEST_ASSERT(0, "Item allocation failed");
            free(keys[i]);
            break;
        }

        *items[i] = (int) i;

        result = generic_hash_table_insert(ht, keys[i], items[i], free_int,
                                           copy_int);
        if (result != 0)
        {
            TEST_ASSERT(0, "Insert operation failed during stress test");
            free(keys[i]);
            free(items[i]);
            break;
        }

        i++;
    }

    TEST_ASSERT(i == num_items, "All items inserted successfully");

    size_t verified_count = 0;
    i = 0;
    while (i < num_items)
    {

        int* check_item = NULL;
        int (*deep_copy)(void*, void**) = NULL;

        result = generic_hash_table_get(ht, keys[i], (void**) &check_item,
                                        &deep_copy);
        if (result == 0 && check_item != NULL && *check_item == (int) i)
        {
            verified_count++;
        }

        i++;
    }

    char log_buffer[128];
    
    TEST_ASSERT(verified_count == num_items, "All items verified successfully");

    i = 0;
    while (i < num_items)
    {

        free(keys[i]);
        free(items[i]);
        i++;
    }

    free(keys);
    free(items);

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
    generic_hash_table_insert_and_get_basic_test();
    generic_hash_table_delete_test();
    generic_hash_table_overwrite_test();
    generic_hash_table_stress_test();

    printf("\n");
    printf("*****************************************\n");
    printf("End Generic Hash Table Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}
