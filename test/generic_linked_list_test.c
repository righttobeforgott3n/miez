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
free_int(void* ptr)
{
    free(ptr);
}

int
compare_int(void* a, void* b)
{
    return *(int*) a - *(int*) b;
}

void
print_int(void* data)
{
    printf("%d ", *(int*) data);
}

void
double_int(void* data)
{
    *(int*) data *= 2;
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
generic_linked_list_iterator_begin_end_test()
{

    TEST_SUITE("Generic Linked List Iterator Begin/End Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    TEST_ASSERT(generic_linked_list_iterator_is_end(begin) == 0,
                "Begin equals end for empty list\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    int values[] = {10, 20, 30};
    size_t i = 0;
    while (i < 3)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created for non-empty list\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created for non-empty list\n");

    TEST_ASSERT(generic_linked_list_iterator_is_end(begin) != 0,
                "Begin not equals end for non-empty list\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(begin, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 10,
                "Begin points to first element\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_next_prev_test()
{

    TEST_SUITE("Generic Linked List Iterator Next/Prev Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 10, "First element is 10\n");

    exit_code = generic_linked_list_iterator_next(iter);
    TEST_ASSERT(!exit_code, "Next succeeded\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 20, "Second element is 20\n");

    exit_code = generic_linked_list_iterator_next(iter);
    TEST_ASSERT(!exit_code, "Next succeeded\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 30, "Third element is 30\n");

    exit_code = generic_linked_list_iterator_prev(iter);
    TEST_ASSERT(!exit_code, "Prev succeeded\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 20,
                "After prev, element is 20\n");

    generic_linked_list_iterator_free(iter);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_reverse_test()
{

    TEST_SUITE("Generic Linked List Iterator Reverse Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator rbegin = NULL;
    exit_code = generic_linked_list_iterator_reverse_begin(ll, &rbegin);
    TEST_ASSERT(!exit_code, "Reverse begin iterator created\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(rbegin, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 50,
                "Reverse begin points to last element (50)\n");

    exit_code = generic_linked_list_iterator_prev(rbegin);
    TEST_ASSERT(!exit_code, "Prev succeeded\n");

    exit_code = generic_linked_list_iterator_get(rbegin, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 40,
                "After prev, element is 40\n");

    exit_code = generic_linked_list_iterator_prev(rbegin);
    TEST_ASSERT(!exit_code, "Prev succeeded\n");

    exit_code = generic_linked_list_iterator_get(rbegin, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 30,
                "After prev, element is 30\n");

    generic_linked_list_iterator_free(rbegin);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_advance_test()
{

    TEST_SUITE("Generic Linked List Iterator Advance Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    size_t i = 0;
    while (i < 10)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    exit_code = generic_linked_list_iterator_advance(iter, 5);
    TEST_ASSERT(!exit_code, "Advance by 5 succeeded\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 60,
                "After advance(5), element is 60\n");

    exit_code = generic_linked_list_iterator_advance(iter, 3);
    TEST_ASSERT(!exit_code, "Advance by 3 succeeded\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 90,
                "After advance(3), element is 90\n");

    generic_linked_list_iterator_free(iter);

    exit_code = generic_linked_list_iterator_reverse_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Reverse iterator created\n");

    exit_code = generic_linked_list_iterator_reverse_advance(iter, 4);
    TEST_ASSERT(!exit_code, "Reverse advance by 4 succeeded\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 60,
                "After reverse_advance(4), element is 60\n");

    generic_linked_list_iterator_free(iter);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_get_test()
{

    TEST_SUITE("Generic Linked List Iterator Get Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {42, 99, 123};
    size_t i = 0;
    while (i < 3)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code, "Get succeeded\n");
    TEST_ASSERT(data == &values[0], "Get returns pointer to original data\n");
    TEST_ASSERT(*(int*) data == 42, "Value is 42\n");

    *(int*) data = 999;
    TEST_ASSERT(values[0] == 999, "Modification through get affects list\n");

    generic_linked_list_iterator end = NULL;
    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    exit_code = generic_linked_list_iterator_get(end, &data);
    TEST_ASSERT(exit_code == 1, "Get on end iterator fails\n");
    TEST_ASSERT(data == NULL, "Data is NULL for end iterator\n");

    generic_linked_list_iterator_free(iter);
    generic_linked_list_iterator_free(end);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_is_valid_test()
{

    TEST_SUITE("Generic Linked List Iterator Is Valid Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30};
    size_t i = 0;
    while (i < 3)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    TEST_ASSERT(generic_linked_list_iterator_is_valid(begin) == 0,
                "Begin iterator is valid\n");

    TEST_ASSERT(generic_linked_list_iterator_is_valid(end) != 0,
                "End iterator is not valid\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_remove_test()
{

    TEST_SUITE("Generic Linked List Iterator Remove Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_int);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t i = 0;
    while (i < 5)
    {
        int value = (i + 1) * 10;
        generic_linked_list_insert_last(ll, &value);
        i++;
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 5, "Size is 5\n");

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    generic_linked_list_iterator_advance(iter, 2);

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 30, "Element at index 2 is 30\n");

    exit_code = generic_linked_list_iterator_remove(iter, NULL);
    TEST_ASSERT(!exit_code, "Remove with auto-free succeeded\n");

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 4, "Size is 4 after removal\n");

    exit_code = generic_linked_list_iterator_get(iter, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 40,
                "Iterator now points to next element (40)\n");

    void* removed_data = NULL;
    exit_code = generic_linked_list_iterator_remove(iter, &removed_data);
    TEST_ASSERT(!exit_code, "Remove with out_data succeeded\n");
    TEST_ASSERT(*(int*) removed_data == 40, "Removed data is 40\n");
    free(removed_data);

    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 3, "Size is 3 after second removal\n");

    generic_linked_list_iterator_free(iter);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_distance_test()
{

    TEST_SUITE("Generic Linked List Iterator Distance Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    size_t distance = 0;
    exit_code = generic_linked_list_iterator_distance(begin, end, &distance);
    TEST_ASSERT(!exit_code, "Distance calculation succeeded\n");
    TEST_ASSERT(distance == 5, "Distance from begin to end is 5\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    exit_code = generic_linked_list_iterator_begin(ll, &end);
    generic_linked_list_iterator_advance(end, 3);

    exit_code = generic_linked_list_iterator_distance(begin, end, &distance);
    TEST_ASSERT(!exit_code, "Distance calculation succeeded\n");
    TEST_ASSERT(distance == 3, "Distance from begin to begin+3 is 3\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    exit_code = generic_linked_list_iterator_begin(ll, &end);

    exit_code = generic_linked_list_iterator_distance(begin, end, &distance);
    TEST_ASSERT(!exit_code, "Distance calculation succeeded\n");
    TEST_ASSERT(distance == 0, "Distance from begin to begin is 0\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_find_test()
{

    TEST_SUITE("Generic Linked List Iterator Find Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;
    generic_linked_list_iterator found = NULL;

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    int target = 30;
    exit_code = generic_linked_list_iterator_find(begin, end, &target,
                                                  compare_int, &found);
    TEST_ASSERT(!exit_code, "Find succeeded\n");
    TEST_ASSERT(generic_linked_list_iterator_is_valid(found) == 0,
                "Found iterator is valid\n");

    void* data = NULL;
    exit_code = generic_linked_list_iterator_get(found, &data);
    TEST_ASSERT(!exit_code && *(int*) data == 30, "Found element is 30\n");

    generic_linked_list_iterator_free(found);

    int not_exist = 999;
    exit_code = generic_linked_list_iterator_find(begin, end, &not_exist,
                                                  compare_int, &found);
    TEST_ASSERT(!exit_code, "Find for non-existent element succeeded\n");
    TEST_ASSERT(generic_linked_list_iterator_is_end(found) == 0,
                "Found iterator equals end (not found)\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_iterator_free(found);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_for_each_test()
{

    TEST_SUITE("Generic Linked List Iterator For Each Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    int values[] = {10, 20, 30, 40, 50};
    size_t i = 0;
    while (i < 5)
    {
        generic_linked_list_insert_last(ll, &values[i]);
        i++;
    }

    generic_linked_list_iterator begin = NULL;
    generic_linked_list_iterator end = NULL;

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    TEST_ASSERT(!exit_code, "Begin iterator created\n");

    exit_code = generic_linked_list_iterator_end(ll, &end);
    TEST_ASSERT(!exit_code, "End iterator created\n");

    printf("  Elements before double: ");
    exit_code = generic_linked_list_iterator_for_each(begin, end, print_int);
    printf("\n");
    TEST_ASSERT(!exit_code, "For each (print) succeeded\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    exit_code = generic_linked_list_iterator_end(ll, &end);

    exit_code = generic_linked_list_iterator_for_each(begin, end, double_int);
    TEST_ASSERT(!exit_code, "For each (double) succeeded\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);

    exit_code = generic_linked_list_iterator_begin(ll, &begin);
    exit_code = generic_linked_list_iterator_end(ll, &end);

    printf("  Elements after double: ");
    exit_code = generic_linked_list_iterator_for_each(begin, end, print_int);
    printf("\n");
    TEST_ASSERT(!exit_code, "For each (print after double) succeeded\n");

    TEST_ASSERT(values[0] == 20, "First element doubled to 20\n");
    TEST_ASSERT(values[1] == 40, "Second element doubled to 40\n");
    TEST_ASSERT(values[2] == 60, "Third element doubled to 60\n");
    TEST_ASSERT(values[3] == 80, "Fourth element doubled to 80\n");
    TEST_ASSERT(values[4] == 100, "Fifth element doubled to 100\n");

    generic_linked_list_iterator_free(begin);
    generic_linked_list_iterator_free(end);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_full_traversal_test()
{

    TEST_SUITE("Generic Linked List Iterator Full Traversal Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    size_t n = 100;
    size_t i = 0;
    while (i < n)
    {
        int* value = malloc(sizeof(int));
        *value = i;
        generic_linked_list_insert_last(ll, value);
        i++;
    }

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    size_t count = 0;
    while (generic_linked_list_iterator_is_end(iter))
    {
        void* data = NULL;
        exit_code = generic_linked_list_iterator_get(iter, &data);
        if (!exit_code && *(int*) data == (int) count)
        {
            count++;
        }
        generic_linked_list_iterator_next(iter);
    }

    TEST_ASSERT(count == n, "Forward traversal visited all elements\n");

    generic_linked_list_iterator_free(iter);

    exit_code = generic_linked_list_iterator_reverse_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Reverse iterator created\n");

    count = 0;
    while (generic_linked_list_iterator_is_begin(iter))
    {
        void* data = NULL;
        exit_code = generic_linked_list_iterator_get(iter, &data);
        if (!exit_code && *(int*) data == (int) (n - 1 - count))
        {
            count++;
        }
        generic_linked_list_iterator_prev(iter);
    }

    TEST_ASSERT(count == n, "Reverse traversal visited all elements\n");

    generic_linked_list_iterator_free(iter);

    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    while (generic_linked_list_iterator_is_end(iter))
    {
        void* data = NULL;
        generic_linked_list_iterator_remove(iter, &data);
        free(data);
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 0, "All elements removed via iterator\n");

    generic_linked_list_iterator_free(iter);
    generic_linked_list_free(ll);

    return 0;
}

int
generic_linked_list_iterator_remove_while_iterating_test()
{

    TEST_SUITE("Generic Linked List Iterator Remove While Iterating Test");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    TEST_ASSERT(!exit_code, "List created\n");

    exit_code = generic_linked_list_set_copy_function(ll, copy_int);
    TEST_ASSERT(!exit_code, "Copy function set\n");

    exit_code = generic_linked_list_set_free_function(ll, free_int);
    TEST_ASSERT(!exit_code, "Free function set\n");

    size_t i = 0;
    while (i < 10)
    {
        int value = i;
        generic_linked_list_insert_last(ll, &value);
        i++;
    }

    generic_linked_list_iterator iter = NULL;
    exit_code = generic_linked_list_iterator_begin(ll, &iter);
    TEST_ASSERT(!exit_code, "Iterator created\n");

    while (generic_linked_list_iterator_is_end(iter))
    {
        void* data = NULL;
        generic_linked_list_iterator_get(iter, &data);

        if (*(int*) data % 2 == 0)
        {
            generic_linked_list_iterator_remove(iter, NULL);
        }
        else
        {
            generic_linked_list_iterator_next(iter);
        }
    }

    size_t size = 0;
    generic_linked_list_size(ll, &size);
    TEST_ASSERT(size == 5, "Size is 5 after removing even numbers\n");

    generic_linked_list_iterator_free(iter);

    exit_code = generic_linked_list_iterator_begin(ll, &iter);

    int expected[] = {1, 3, 5, 7, 9};
    i = 0;
    while (generic_linked_list_iterator_is_end(iter) && i < 5)
    {
        void* data = NULL;
        generic_linked_list_iterator_get(iter, &data);
        TEST_ASSERT(*(int*) data == expected[i], "Element matches expected\n");
        generic_linked_list_iterator_next(iter);
        i++;
    }

    generic_linked_list_iterator_free(iter);
    generic_linked_list_free(ll);

    return 0;
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    printf("\n");
    printf("*****************************************\n");
    printf("Begin Generic Linked List Test Suite\n");
    printf("*****************************************\n");

    generic_linked_list_new_and_free_test();
    generic_linked_list_borrow_mode_test();
    generic_linked_list_full_ownership_test();
    generic_linked_list_copy_no_free_test();
    generic_linked_list_mixed_ownership_test();
    generic_linked_list_error_cases_test();
    generic_linked_list_stress_test();
    generic_linked_list_alternating_operations_test();

    generic_linked_list_iterator_begin_end_test();
    generic_linked_list_iterator_next_prev_test();
    generic_linked_list_iterator_reverse_test();
    generic_linked_list_iterator_advance_test();
    generic_linked_list_iterator_get_test();
    generic_linked_list_iterator_is_valid_test();
    generic_linked_list_iterator_remove_test();
    generic_linked_list_iterator_distance_test();
    generic_linked_list_iterator_find_test();
    generic_linked_list_iterator_for_each_test();
    generic_linked_list_iterator_full_traversal_test();
    generic_linked_list_iterator_remove_while_iterating_test();

    printf("\n");
    printf("*****************************************\n");
    printf("End Generic Linked List Test Suite\n");
    printf("*****************************************\n");

    printf("Tests passed: %d\nTests failed: %d\n", stats.passed, stats.failed);

    return stats.failed;
}
