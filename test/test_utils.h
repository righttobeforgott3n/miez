#include <stdio.h>

typedef struct
{
    int passed;
    int failed;
} test_stats_t;

static test_stats_t stats = { 0, 0 };

#define TEST_ASSERT(condition, message)                                        \
    {                                                                          \
        if (condition)                                                         \
        {                                                                      \
            printf("  ✓ PASS: %s\n", message);                                 \
            stats.passed++;                                                    \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("  ✗ FAIL: %s\n", message);                                 \
            stats.failed++;                                                    \
        }                                                                      \
    }

#define TEST_SUITE(name) printf("\n[TEST SUITE] %s\n", name)