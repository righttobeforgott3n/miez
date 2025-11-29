#include "generic_linked_list.h"
#include "test_utils.h"
#include <stdio.h>
#include <stdlib.h>

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
generic_linked_list_insert_and_remove_no_ownership_test(size_t n_elements)
{

    TEST_SUITE("Generic Linked List Remove Test\n");

    generic_linked_list ll = NULL;
    int exit_code = generic_linked_list_new(&ll);
    if (exit_code)
    {
        return 1;
    }

    size_t ll_size = 1;
    exit_code = generic_linked_list_size(ll, &ll_size);

    TEST_ASSERT(!exit_code, "Get Size operation ended with success\n");
    TEST_ASSERT(!ll_size, "Generic Linked List size: zero\n");

    void (*free_function)(void*) = free;
    exit_code = generic_linked_list_get_free_function(ll, &free_function);

    TEST_ASSERT(!exit_code, "Get Free Function operation ended with success\n");
    TEST_ASSERT(!free_function, "Free Function is set to NULL\n");

    int* elements[n_elements];
    size_t i = 0;
    while (n_elements > i)
    {

        elements[i] = malloc(sizeof(int));
        *elements[i] = i;

        exit_code = generic_linked_list_insert_last(ll, elements[i]);
        if (exit_code)
        {

            TEST_ASSERT(0, "Error in generic_linked_list_insert_last\n");

            size_t ii = 0;
            while (ii < i)
            {
                free(elements[ii]);
                ii++;
            }

            generic_linked_list_free(ll);

            return 1;
        }

        i++;
    }

    generic_linked_list_size(ll, &ll_size);

    TEST_ASSERT(n_elements == ll_size, "All the elements have been inserted\n");

    // @todo remove all the elements.
    
    i = 0;
    while (i < n_elements)
    {
        free(elements[i]);
        i++;
    }
    exit_code = generic_linked_list_free(ll);

    return 0;
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    generic_linked_list_new_and_free_test();
    generic_linked_list_insert_and_remove_no_ownership_test(1000);

    return stats.failed;
}