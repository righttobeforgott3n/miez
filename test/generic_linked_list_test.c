#include "generic_linked_list.h"
#include "test_utils.h"
#include <stdio.h>

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
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    generic_linked_list_new_and_free_test();

    return stats.failed;
}