
#include "tests/assert.h"
#include "util/hash.h"

DEFINE_TEST(testEqualHashBytes, "-", {
    char test_a[] = { 1, 2, 3, 4 };
    char test_b[] = { 1, 2, 3, 4 };
    ASSERT_EQUAL(hashBytes(test_a, 4), hashBytes(test_b, 4));
})

DEFINE_TEST(testEqualHashInt, "-", {
    ASSERT_EQUAL(hashInt(123456), hashInt(123456));
})

DEFINE_TEST(testEqualHashCString, "-", {
    ASSERT_EQUAL(hashCString("Hello world"), hashCString("Hello world"));
})

DEFINE_TEST(testEqualHashCombine, "-", {
    ASSERT_EQUAL(hashCombine(123, 567), hashCombine(123, 567));
})

DEFINE_TEST(testEqualHashString, "-", {
    ASSERT_EQUAL(hashString(str("Hello world")), hashString(str("Hello world")));
})

DEFINE_TEST(testUnequalHashBytes, "-", {
    char test_a[] = { 1, 2, 2, 4 };
    char test_b[] = { 1, 2, 3, 4 };
    ASSERT_UNEQUAL(hashBytes(test_a, 4), hashBytes(test_b, 4));
})

DEFINE_TEST(testUnequalHashInt, "-", {
    ASSERT_UNEQUAL(hashInt(123456), hashInt(123466));
})

DEFINE_TEST(testUnequalHashCString, "-", {
    ASSERT_UNEQUAL(hashCString("Hello world."), hashCString("Hello world"));
})

DEFINE_TEST(testUnequalHashCombine, "-", {
    ASSERT_UNEQUAL(hashCombine(123, 567), hashCombine(567, 123));
})

DEFINE_TEST(testUnequalHashString, "-", {
    ASSERT_UNEQUAL(hashString(str("Hello world")), hashString(str("Hello World")));
})

