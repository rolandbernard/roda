
#include "tests/test.h"
#include "tests/assert.h"

DEFINE_TEST(testEmptyTest, "Empty tests should be successful", {
})

DEFINE_TEST(testAssertTest, "True assert should not cause failure", {
    ASSERT_TRUE(true);
    ASSERT_NULL(NULL);
    ASSERT_NOT_NULL((void*)1);
    ASSERT_EQUAL(42, 42);
    ASSERT_UNEQUAL(0, 1);
})

