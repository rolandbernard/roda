
#include "tests/assert.h"
#include "const/bigint.h"

#define ASSERT_BIGINT_STR(INT, STR) {       \
    String str = stringForBigInt(INT, 10);  \
    ASSERT_STR_EQUAL(cstr(str), STR);       \
    freeString(str);                        \
}

#define ASSERT_BIGINT(INT, NUM) \
    ASSERT_EQUAL(intMaxForBigInt(INT), NUM);

DEFINE_TEST(testCreateBigInt, "-", {
    BigInt* a = createBigInt();
    ASSERT_BIGINT(a, 0);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 0);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromSmall, "-", {
    BigInt* a = createBigIntFrom(42);
    ASSERT_BIGINT(a, 42);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromLarge, "-", {
    BigInt* a = createBigIntFrom(1234567890123456789);
    ASSERT_BIGINT(a, 1234567890123456789);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromSmallNeg, "-", {
    BigInt* a = createBigIntFrom(-123);
    ASSERT_BIGINT(a, -123);
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromLargeNeg, "-", {
    BigInt* a = createBigIntFrom(-98765432100000000);
    ASSERT_BIGINT(a, -98765432100000000);
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringSmall, "-", {
    BigInt* a = createBigIntFromString(str("42"), 10);
    ASSERT_BIGINT(a, 42);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLarge, "-", {
    BigInt* a = createBigIntFromString(str("1234567890123456789"), 10);
    ASSERT_BIGINT(a, 1234567890123456789);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringSmallNeg, "-", {
    BigInt* a = createBigIntFromString(str("-123"), 10);
    ASSERT_BIGINT(a, -123);
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLargeNeg, "-", {
    BigInt* a = createBigIntFromString(str("-98765432100000000"), 10);
    ASSERT_BIGINT(a, -98765432100000000);
    ASSERT_TRUE(a->negative);
    ASSERT_EQUAL(a->size, 2);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringHex, "-", {
    BigInt* a = createBigIntFromString(str("fa03"), 16);
    ASSERT_BIGINT(a, 64003);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringBin, "-", {
    BigInt* a = createBigIntFromString(str("1000101111010011"), 2);
    ASSERT_BIGINT(a, 35795);
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 1);
    freeBigInt(a);
})

DEFINE_TEST(testCreateBigIntFromStringLarge2, "-", {
    BigInt* a = createBigIntFromString(str("123456789abcdef00000000000000000000000000"), 16);
    ASSERT_BIGINT_STR(a, "1662864085140938409653700456423626137158360760320");
    ASSERT_FALSE(a->negative);
    ASSERT_EQUAL(a->size, 6);
    freeBigInt(a);
})

DEFINE_TEST(testCopyBigIntZero, "-", {
    BigInt* a = createBigInt();
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, 0);
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 0);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromSmall, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, 42);
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 1);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromLarge, "-", {
    BigInt* a = createBigIntFrom(1234567890123456789);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, 1234567890123456789);
    ASSERT_FALSE(b->negative);
    ASSERT_EQUAL(b->size, 2);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromSmallNeg, "-", {
    BigInt* a = createBigIntFrom(-123);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, -123);
    ASSERT_TRUE(b->negative);
    ASSERT_EQUAL(b->size, 1);
    freeBigInt(b);
})

DEFINE_TEST(testCopyBigIntFromLargeNeg, "-", {
    BigInt* a = createBigIntFrom(-98765432100000000);
    BigInt* b = copyBigInt(a);
    freeBigInt(a);
    ASSERT_BIGINT(b, -98765432100000000);
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
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-43);
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

DEFINE_TEST(testNegBigIntZero, "-", {
    BigInt* a = createBigInt();
    BigInt* b = negBigInt(a);
    ASSERT_BIGINT(b, 0);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testNegBigIntPos, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = negBigInt(a);
    ASSERT_BIGINT(b, -42);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testNegBigIntNeg, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = negBigInt(a);
    ASSERT_BIGINT(b, 42);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testAbsBigIntZero, "-", {
    BigInt* a = createBigInt();
    BigInt* b = absBigInt(a);
    ASSERT_BIGINT(b, 0);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testAbsBigIntPos, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = absBigInt(a);
    ASSERT_BIGINT(b, 42);
    ASSERT_EQUAL(compareBigInt(a, b), 0);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testAbsBigIntNeg, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = absBigInt(a);
    ASSERT_BIGINT(b, 42);
    freeBigInt(a);
    freeBigInt(b);
})

DEFINE_TEST(testAddBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(42);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT(c, 84);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt2, "-", {
    BigInt* a = createBigIntFrom(9145678912345678901);
    BigInt* b = createBigIntFrom(9145678912345678901);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT_STR(c, "18291357824691357802");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt3, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT(c, -22);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt4, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-20);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT(c, 22);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt5, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-20);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT(c, -62);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt6, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT_STR(c, "14966048989831189770931389386893450031259204");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAddBigInt7, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = addBigInt(a, b);
    ASSERT_BIGINT_STR(c, "14966505724369675501907842622245151975259546");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(42);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT(c, 0);
    ASSERT_EQUAL(c->size, 0);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt2, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-42);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT(c, 84);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt3, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-20);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT(c, -22);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt4, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT(c, -62);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt5, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT_STR(c, "14966048989831189770931389386893450031259204");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testSubBigInt6, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = subBigInt(a, b);
    ASSERT_BIGINT_STR(c, "14966505724369675501907842622245151975259546");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT(c, 840);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt2, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-987654321);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT(c, -41481481482);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt3, "-", {
    BigInt* a = createBigIntFrom(-123456789);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT(c, -2469135780);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt4, "-", {
    BigInt* a = createBigIntFrom(-123456789123456789);
    BigInt* b = createBigIntFrom(-987654321987654321);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT_STR(c, "121932631356500531347203169112635269");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt5, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT_STR(c, "3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt6, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT_STR(c, "-3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt7, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT_STR(c, "-3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt8, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT_STR(c, "3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, 2);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt2, "-", {
    BigInt* a = createBigIntFrom(987654321987654321);
    BigInt* b = createBigIntFrom(-1234);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, -800368170168277);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt3, "-", {
    BigInt* a = createBigIntFrom(-123456789123456789);
    BigInt* b = createBigIntFrom(42);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, -2939447360082304);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt4, "-", {
    BigInt* a = createBigIntFrom(-123456789987654321);
    BigInt* b = createBigIntFrom(-54321);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, 2272726753698);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt5, "-", {
    BigInt* a = createBigIntFrom(-54320);
    BigInt* b = createBigIntFrom(-54321);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, 0);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt6, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, 65536);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt7, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, -65536);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt8, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, 65536);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testDivBigInt9, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = divBigInt(a, b);
    ASSERT_BIGINT(c, -65536);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(20);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, 2);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt2, "-", {
    BigInt* a = createBigIntFrom(12345678987654321);
    BigInt* b = createBigIntFrom(-12345);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, 696);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt3, "-", {
    BigInt* a = createBigIntFrom(-98765432123456789);
    BigInt* b = createBigIntFrom(987654321);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, -23456789);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt4, "-", {
    BigInt* a = createBigIntFrom(-987654321);
    BigInt* b = createBigIntFrom(-987654321123456789);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, -987654321);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt5, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, 52719);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt6, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, -52719);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt7, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, 52719);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testRemBigInt8, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* c = remBigInt(a, b);
    ASSERT_BIGINT(c, -52719);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAndBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = andBigInt(a, b);
    ASSERT_BIGINT(c, 10);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAndBigInt2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = andBigInt(a, b);
    ASSERT_BIGINT(c, 18);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAndBigInt3, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = andBigInt(a, b);
    ASSERT_BIGINT(c, 34);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testAndBigInt4, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = andBigInt(a, b);
    ASSERT_BIGINT(c, -58);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testOrBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = orBigInt(a, b);
    ASSERT_BIGINT(c, 58);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testOrBigInt2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = orBigInt(a, b);
    ASSERT_BIGINT(c, -34);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testOrBigInt3, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = orBigInt(a, b);
    ASSERT_BIGINT(c, -18);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testOrBigInt4, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = orBigInt(a, b);
    ASSERT_BIGINT(c, -10);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testXorBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = xorBigInt(a, b);
    ASSERT_BIGINT(c, 48);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testXorBigInt2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(26);
    BigInt* c = xorBigInt(a, b);
    ASSERT_BIGINT(c, -52);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testXorBigInt3, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = xorBigInt(a, b);
    ASSERT_BIGINT(c, -52);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testXorBigInt4, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* b = createBigIntFrom(-26);
    BigInt* c = xorBigInt(a, b);
    ASSERT_BIGINT(c, 48);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testNotBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* c = notBigInt(a);
    ASSERT_BIGINT(c, -43);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testNotBigInt2, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* c = notBigInt(a);
    ASSERT_BIGINT(c, 41);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftLeftBigInt1, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* c = shiftLeftBigInt(a, 0);
    ASSERT_BIGINT(c, 42);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftLeftBigInt2, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* c = shiftLeftBigInt(a, 32);
    ASSERT_BIGINT(c, 180388626432);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftLeftBigInt3, "-", {
    BigInt* a = createBigIntFrom(42);
    BigInt* c = shiftLeftBigInt(a, 1234);
    ASSERT_BIGINT_STR(c, "12424071433540142420521877220076350813026266859685665674061360696127608733442034818964261164431878107830633205880980151711074477771876033959355142924279319687537208647948638557706820526101026562169644639169603467248951302514030775012441411551731304491930775759074912082229886225695359354289776176171318118053274739978971350726662145988449999470415097463527440874884697161728");
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftLeftBigInt4, "-", {
    BigInt* a = createBigIntFrom(-42);
    BigInt* c = shiftLeftBigInt(a, 123);
    ASSERT_BIGINT_STR(c, "-446620606583731733295679172254195777536");
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftRightBigInt1, "-", {
    BigInt* a = createBigIntFrom(12345678987654321);
    BigInt* c = shiftRightBigInt(a, 32);
    ASSERT_BIGINT(c, 2874452);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftRightBigInt2, "-", {
    BigInt* a = createBigIntFrom(12345678987654321);
    BigInt* c = shiftRightBigInt(a, 42);
    ASSERT_BIGINT(c, 2807);
    freeBigInt(a);
    freeBigInt(c);
})

DEFINE_TEST(testShiftRightBigInt3, "-", {
    BigInt* a = createBigIntFromString(str("12424071433540142420521877220076350813026266859685665674061360696127608733442034818964261164431878107830633205880980151711074477771876033959355142924279319687537208647948638557706820526101026562169644639169603467248951302514030775012441411551731304491930775759074912082229886225695359354289776176171318118053274739978971350726662145988449999470415097463527440874884697161728"), 10);
    BigInt* c = shiftRightBigInt(a, 1234);
    ASSERT_BIGINT(c, 42);
    freeBigInt(a);
    freeBigInt(c);
})

