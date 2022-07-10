
#include "tests/assert.h"
#include "util/macro.h"

DEFINE_TEST(testStringify, "-", {
    ASSERT_STR_EQUAL(STRINGIFY(Hello World), "Hello World");
})

DEFINE_TEST(testConcat, "-", {
    int test = 1;
    int ing = 2;
    int testing = test + ing + 39;
    ASSERT_EQUAL(CONCAT(test, ing), 42);
})

