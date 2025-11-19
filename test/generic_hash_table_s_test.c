#include "generic_hash_table_s.h"
#include "test_utils.h"
#include <stdio.h>

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
    exit_code =
        generic_hash_table_s_insert(hts, (void*) key_0, (void*) &value_0);
    sprintf(message,
            "generic_hash_table_s_insert - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);

    exit_code = generic_hash_table_s_free(hts);
    sprintf(message, "generic_hash_table_s_free - Exit Code should be zero: %d",
            exit_code);
    TEST_ASSERT(!exit_code, message);
}

int
main(int argc __attribute__((unused)), char** argv __attribute__((unused)))
{

    TEST_SUITE("Begin Hash Table Syn Test");

    generic_hash_table_s_new_and_free();
    generic_hash_table_s_insert_test();

    TEST_SUITE("End Hash Table Syn Test");

    return 0;
}