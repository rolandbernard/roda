
#include "tests/test.h"
#include "tests/assert.h"

DEFINE_TEST(testEmptyTest, "Empty tests should be successful", {
})

DEFINE_TEST(testAssertTest, "True assert should not cause failure", {
    ASSERT_TRUE(true);
})

