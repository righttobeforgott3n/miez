#include "generic_linked_list.h"
#include "test_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
copy_int(void* src, void** dst)
{

    if (!src || !dst)
    {
        return 1;
    }

    *dst = malloc(sizeof(int));
    if (!*dst)
    {
        return -1;
    }

    *((int*) *dst) = *((int*) src);

    return 0;
}

int
copy_string(void* src, void** dst)
{

    if (!src || !dst)
    {
        return 1;
    }

    char* p_src = (char*) src;
    size_t len = 0;
    while (p_src[len])
    {
        len++;
    }

    *dst = malloc(len + 1);
    if (!*dst)
    {
        return -1;
    }

    char* p_dst = (char*) *dst;
    size_t i = 0;
    while (i <= len)
    {
        p_dst[i] = p_src[i];
        i++;
    }

    return 0;
}

void
free_wrapper(void* ptr)
{
    free(ptr);
}

int
generic_linked_list_new_and_free_test()
{

    TEST_SUITE("Generic Linked List New and Free Test");

    generic_linked_list* ll_null_0 = NULL;
    int exit_code = generic_linked_list_new(ll_null_0);

    TEST_ASSERT(exit_code == 1, "Nullity check passed on creation\n");

    generic_linked_list ll;
    exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "Generic Linked List created with success\n");

    size_t ll_size = 1;
    exit_code = generic_linked_list_size(ll, &ll_size);

    TEST_ASSERT(!ll_size, "Generic Linked List size equals zero\n");

    generic_linked_list ll_null_1 = NULL;
    exit_code = generic_linked_list_free(ll_null_1);

    TEST_ASSERT(exit_code == 1, "Nullity check on destruction\n");

    exit_code = generic_linked_list_free(ll);

    TEST_ASSERT(!exit_code,
                "Generic Linked List instance freed with success\n");
    ll = NULL;

    return 0;
}

int
generic_linked_list_borrow_mode_test()
{

    TEST_SUITE("Generic Linked List Borrow Mode Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    void (*free_fn)(void*) = free_wrapper;
    int (*copy_fn)(void*, void**) = copy_int;

    exit_code = generic_linked_list_get_free_function(ll, &free_fn);
    TEST_ASSERT(!exit_code && !free_fn, "Free function is NULL\n");

    exit_code = generic_linked_list_get_copy_function(ll, &copy_fn);
    TEST_ASSERT(!exit_code && !copy_fn, "Copy function is NULL\n");

    int values[5] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        exit_code = generic_linked_list_insert_last(ll, &values[i]);
        TEST_ASSERT(!exit_code, "Insert last succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 5, "Size is 5\n");

    void* data = NULL;
    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code, "Remove first succeeded\n");
    TEST_ASSERT(*(int*) data == 10, "First element is 10\n");

    exit_code = generic_linked_list_remove_last(ll, &data);
    TEST_ASSERT(!exit_code, "Remove last succeeded\n");
    TEST_ASSERT(*(int*) data == 50, "Last element is 50\n");

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 3, "Size is 3 after removals\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed\n");

    return 0;
}

int
generic_linked_list_full_ownership_test()
{

    TEST_SUITE("Generic Linked List Full Ownership Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t i = 0;
    while (i < 100)
    {
        int value = i * 10;
        exit_code = generic_linked_list_insert_last(ll, &value);
        TEST_ASSERT(!exit_code, "Insert last with copy succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 100, "Size is 100\n");

    i = 0;
    while (i < 50)
    {
        exit_code = generic_linked_list_remove_first(ll, NULL);
        TEST_ASSERT(!exit_code, "Remove first with auto-free succeeded\n");
        i++;
    }

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 50, "Size is 50 after removals\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed with remaining items\n");

    return 0;
}

int
generic_linked_list_copy_no_free_test()
{

    TEST_SUITE("Generic Linked List Copy Without Free Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_string);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    char* strings[] = {"hello", "world", "test", "data"};
    size_t i = 0;
    while (i < 4)
    {
        exit_code = generic_linked_list_insert_first(ll, strings[i]);
        TEST_ASSERT(!exit_code, "Insert first with copy succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 4, "Size is 4\n");

    void* data = NULL;
    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code, "Remove first succeeded\n");
    TEST_ASSERT(strcmp((char*) data, "data") == 0, "First element is 'data'\n");
    free(data);

    exit_code = generic_linked_list_remove_last(ll, &data);
    TEST_ASSERT(!exit_code, "Remove last succeeded\n");
    TEST_ASSERT(strcmp((char*) data, "hello") == 0,
                "Last element is 'hello'\n");
    free(data);

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 2, "Size is 2\n");

    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code, "Remove succeeded\n");
    free(data);

    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code, "Remove succeeded\n");
    free(data);

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "Empty list freed\n");

    return 0;
}

int
generic_linked_list_mixed_ownership_test()
{

    TEST_SUITE("Generic Linked List Mixed Ownership Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t i = 0;
    while (i < 10)
    {
        int value = i;
        exit_code = generic_linked_list_insert_last(ll, &value);
        TEST_ASSERT(!exit_code, "Insert succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 10, "Size is 10\n");

    void* data = NULL;
    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 0, "Manual remove first\n");
    free(data);

    exit_code = generic_linked_list_remove_last(ll, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 9, "Manual remove last\n");
    free(data);

    exit_code = generic_linked_list_remove_first(ll, NULL);
    TEST_ASSERT(!exit_code, "Auto-free remove first\n");

    exit_code = generic_linked_list_remove_last(ll, NULL);
    TEST_ASSERT(!exit_code, "Auto-free remove last\n");

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 6, "Size is 6 after mixed removals\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed with remaining items\n");

    return 0;
}

int
generic_linked_list_error_cases_test()
{

    TEST_SUITE("Generic Linked List Error Cases Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    int value = 42;
    exit_code = generic_linked_list_insert_first(ll, &value);
    TEST_ASSERT(!exit_code, "Insert succeeded\n");

    exit_code = generic_linked_list_remove_first(ll, NULL);
    TEST_ASSERT(exit_code == 1,
                "Remove with NULL out_data and no free function fails\n");

    void* data = NULL;
    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code, "Remove succeeded\n");

    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code && data == NULL, "Remove from empty list\n");

    exit_code = generic_linked_list_remove_last(ll, &data);
    TEST_ASSERT(!exit_code && data == NULL, "Remove last from empty list\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed\n");

    return 0;
}

int
generic_linked_list_stress_test()
{

    TEST_SUITE("Generic Linked List Stress Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t n = 10000;
    size_t i = 0;
    while (i < n)
    {
        int value = i;
        if (i % 2 == 0)
        {
            exit_code = generic_linked_list_insert_first(ll, &value);
        }
        else
        {
            exit_code = generic_linked_list_insert_last(ll, &value);
        }

        if (exit_code)
        {
            TEST_ASSERT(0, "Insert failed\n");
            generic_linked_list_free(ll);
            return 1;
        }

        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == n, "Size matches inserted elements\n");

    i = 0;
    while (i < n / 2)
    {
        if (i % 2 == 0)
        {
            exit_code = generic_linked_list_remove_first(ll, NULL);
        }
        else
        {
            void* data = NULL;
            exit_code = generic_linked_list_remove_last(ll, &data);
            free(data);
        }

        if (exit_code)
        {
            TEST_ASSERT(0, "Remove failed\n");
            generic_linked_list_free(ll);
            return 1;
        }

        i++;
    }

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == n / 2, "Size is half after removals\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed\n");

    return 0;
}

int
generic_linked_list_alternating_operations_test()
{

    TEST_SUITE("Generic Linked List Alternating Operations Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);

    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_wrapper);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t n = 1000;
    size_t i = 0;
    while (i < n)
    {
        int value = i;
        exit_code = generic_linked_list_insert_last(ll, &value);
        TEST_ASSERT(!exit_code, "Insert succeeded\n");
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == n, "Size is 1000\n");

    void* data = NULL;
    exit_code = generic_linked_list_remove_first(ll, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 0, "First element is 0\n");
    free(data);

    exit_code = generic_linked_list_remove_last(ll, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 999, "Last element is 999\n");
    free(data);

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 998, "Size is 998 after two removals\n");

    i = 0;
    while (i < 998)
    {
        exit_code = generic_linked_list_remove_first(ll, NULL);
        if (exit_code)
        {
            TEST_ASSERT(0, "Remove failed\n");
            generic_linked_list_free(ll);
            return 1;
        }
        i++;
    }

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 0, "List is empty\n");

    exit_code = generic_linked_list_free(ll);
    TEST_ASSERT(!exit_code, "List freed\n");

    return 0;
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    generic_linked_list_new_and_free_test();
    generic_linked_list_borrow_mode_test();
    generic_linked_list_full_ownership_test();
    generic_linked_list_copy_no_free_test();
    generic_linked_list_mixed_ownership_test();
    generic_linked_list_error_cases_test();
    generic_linked_list_stress_test();
    generic_linked_list_alternating_operations_test();

    return stats.failed;
}
