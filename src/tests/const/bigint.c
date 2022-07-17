
#include "tests/assert.h"
#include "const/bigint.h"

#define ASSERT_BIGINT(INT, STR) {           \
    String str = stringForBigInt(INT, 10);  \
    ASSERT_STR_EQUAL(cstr(str), STR);       \
    freeString(str);                        \
}

DEFINE_TEST(testCreateBigInt, "-", {
    BigInt* a = createBigInt();
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromSmall, "-", {
    BigInt* a = createBigIntFrom(42);
    ASSERT_BIGINT(a, "42");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromLarge, "-", {
    BigInt* a = createBigIntFrom(1234567890123456789);
    ASSERT_BIGINT(a, "1234567890123456789");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromSmallNeg, "-", {
    BigInt* a = createBigIntFrom(-123);
    ASSERT_BIGINT(a, "-123");
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromLargeNeg, "-", {
    BigInt* a = createBigIntFrom(-98765432100000000);
    ASSERT_BIGINT(a, "-98765432100000000");
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringSmall, "-", {
    BigInt* a = createBigIntFromString(str("42"), 10);
    ASSERT_BIGINT(a, "42");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLarge, "-", {
    BigInt* a = createBigIntFromString(str("1234567890123456789"), 10);
    ASSERT_BIGINT(a, "1234567890123456789");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringSmallNeg, "-", {
    BigInt* a = createBigIntFromString(str("-123"), 10);
    ASSERT_BIGINT(a, "-123");
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLargeNeg, "-", {
    BigInt* a = createBigIntFromString(str("-98765432100000000"), 10);
    ASSERT_BIGINT(a, "-98765432100000000");
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringHex, "-", {
    BigInt* a = createBigIntFromString(str("fa03"), 16);
    ASSERT_BIGINT(a, "64003");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringBin, "-", {
    BigInt* a = createBigIntFromString(str("1000101111010011"), 2);
    ASSERT_BIGINT(a, "35795");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLarge2, "-", {
    BigInt* a = createBigIntFromString(str("123456789abcdef00000000000000000000000000"), 16);
    ASSERT_BIGINT(a, "1662864085140938409653700456423626137158360760320");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 6);
    freeBigInt(a);
})

