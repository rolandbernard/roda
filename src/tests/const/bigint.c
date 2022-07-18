
#include "tests/assert.h"
#include "const/bigint.h"

#define ASSERT_BIGINT(INT, STR) {           \
    String str = stringForBigInt(INT, 10);  \
    ASSERT_STR_EQUAL(cstr(str), STR);       \
    freeString(str);                        \
}

DEFINE_TEST(testCreateBigInt, "-", {
    BigInt* a = createBigInt();
    ASSERT_BIGINT(a, "0");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 0);
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

DEFINE_TEST(testCopyBigIntZero, "-", {
    BigInt* a = createBigInt();
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, "0");
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 0);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromSmall, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, "42");
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 1);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromLarge, "-", {
    BigInt* a = createBigIntFrom(1234567890123456789);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, "1234567890123456789");
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 2);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromSmallNeg, "-", {
    BigInt* a = createBigIntFrom(-123);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, "-123");
    ASSERT_TRUE(b->negative);
    ASSERT_EQUAL(b->size, 1);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromLargeNeg, "-", {
    BigInt* a = createBigIntFrom(-98765432100000000);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, "-98765432100000000");
    ASSERT_TRUE(b->negative);
    ASSERT_EQUAL(b->size, 2);
    freeBigInt(b);
})

DEFINE_TEST(testSignOfBigIntZero, "-", {
    BigInt* a = createBigInt();
    ASSERT_EQUAL(signOfBigInt(a), 0);
    freeBigInt(a);
})

DEFINE_TEST(testSignOfBigIntPos, "-", {
    BigInt* a = createBigIntFrom(42);
    ASSERT_EQUAL(signOfBigInt(a), 1);
    freeBigInt(a);
})

DEFINE_TEST(testSignOfBigIntNeg, "-", {
    BigInt* a = createBigIntFrom(-123456789);
    ASSERT_EQUAL(signOfBigInt(a), -1);
    freeBigInt(a);
})

DEFINE_TEST(testIsZero, "-", {
    BigInt* a = createBigInt();
    ASSERT_TRUE(isZero(a));
    freeBigInt(a);
})

DEFINE_TEST(testIsZeroPos, "-", {
    BigInt* a = createBigIntFrom(42);
    ASSERT_FALSE(isZero(a));
    freeBigInt(a);
})

DEFINE_TEST(testIsZeroNeg, "-", {
    BigInt* a = createBigIntFrom(-123456789);
    ASSERT_FALSE(isZero(a));
    freeBigInt(a);
})

DEFINE_TEST(testCompareBigIntEqual1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(42);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntEqual2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-42);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntEqual3, "-", {
    BigInt* a = createBigIntFromString(str("0123456789abcdefghijklmnopqrstuvwxyz"), 36);
    BigInt* b = createBigIntFromString(str("123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 36);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntLess1, "-", {
    BigInt* a = createBigIntFrom(41);
    BigInt* b = createBigIntFrom(42);
    ASSERT_EQUAL(compareBigInt(a, b), -1);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntLess2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(41);
    ASSERT_EQUAL(compareBigInt(a, b), -1);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntLess3, "-", {
    BigInt* a = createBigIntFromString(str("-0123456789abcdefghijklmnopqrstuvwxyz"), 36);
    BigInt* b = createBigIntFromString(str("-123456789ABCDEFGHIJKLMNOPQRSTUVWXY"), 36);
    ASSERT_EQUAL(compareBigInt(a, b), -1);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntMore1, "-", {
    BigInt* a = createBigIntFrom(43);
    BigInt* b = createBigIntFrom(42);
    ASSERT_EQUAL(compareBigInt(a, b), 1);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntMore2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(43);
    ASSERT_EQUAL(compareBigInt(a, b), 1);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testCompareBigIntMore3, "-", {
    BigInt* a = createBigIntFromString(str("-0123456789abcdefghijklmnopqrstuvwxy"), 36);
    BigInt* b = createBigIntFromString(str("-123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 36);
    ASSERT_EQUAL(compareBigInt(a, b), 1);
    freeBigInt(a);
    freeBigInt(b);
})
