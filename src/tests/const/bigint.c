
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
    ASSERT_TRUE(isBigIntZero(a));
    freeBigInt(a);
})

DEFINE_TEST(testIsZeroPos, "-", {
    BigInt* a = createBigIntFrom(42);
    ASSERT_FALSE(isBigIntZero(a));
    freeBigInt(a);
})

DEFINE_TEST(testIsZeroNeg, "-", {
    BigInt* a = createBigIntFrom(-123456789);
    ASSERT_FALSE(isBigIntZero(a));
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

DEFINE_TEST(testAddBigInt8, "-", {
    BigInt* a = createBigIntFrom(4294967295);
    BigInt* c = addBigInt(a, a);
    ASSERT_BIGINT_STR(c, "8589934590");
    freeBigInt(a);
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

DEFINE_TEST(testMulBigInt9, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* n = createBigIntFrom(1);
    for (size_t i = 0; i < 10; i++) {
        BigInt* nn = mulBigInt(n, a);
        freeBigInt(n);
        n = nn;
    }
    freeBigInt(a);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* m = createBigIntFrom(1);
    for (size_t i = 0; i < 10; i++) {
        BigInt* mm = mulBigInt(m, b);
        freeBigInt(m);
        m = mm;
    }
    freeBigInt(b);
    BigInt* c = mulBigInt(n, m);
    ASSERT_BIGINT_STR(c, "217508624057813131549155851881461695469597637317736912272709573102411904067361947208499227536292620333034386882841303431942718249874095538876607760187803472373721875699710595347873670453620114942222822331878647291624265481684197376075898076064589999622470238948936453249410166081685720560924493765751339090838692237180175263239536023501057606433389884015132665735879562618010507233775990020352354762637193776340845419203939580640080136501220980571245537852949336540608441848421140647089975112454398901006530160609741692902018243997107594896273809154025070992802543691180331186227338900753582675053722334316140965712566678901919781350796811737638393868126897786329305201094087921428545943301387275379398085994559123917799825686020488311384460944866347571296752002251411776273242364343474264387623406946659088134765625");
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt10, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("0"), 16);
    BigInt* c = mulBigInt(a, b);
    ASSERT_BIGINT(c, 0);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt11, "-", {
    BigInt* a = createBigIntFromString(str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* b = createBigIntFromString(str("0"), 16);
    BigInt* c = mulBigInt(b, a);
    ASSERT_BIGINT(c, 0);
    freeBigInt(a);
    freeBigInt(b);
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt12, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* n = createBigIntFrom(1);
    for (size_t i = 0; i < 10; i++) {
        BigInt* nn = mulBigInt(n, a);
        freeBigInt(n);
        n = nn;
    }
    freeBigInt(a);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* m = createBigIntFrom(1);
    for (size_t i = 0; i < 20; i++) {
        BigInt* mm = mulBigInt(m, b);
        freeBigInt(m);
        m = mm;
    }
    freeBigInt(b);
    BigInt* c = mulBigInt(n, m);
    ASSERT_BIGINT_STR(c, "83910260336482200587328407316528466200710664827330427597248899706087058343113104783347684954300253413692447983297891898659794240959285105469599304016445391044739088751579200140172982077171724978024245150131787193715173793064185274454394241030903827011018862311494923168459444634394864887150786907085955579179212866037414481046286992500260667310401796624893809656799866131100746597430944460877177315771903165575842220314260626251553587017167702065532018955969047918191871027256830170484457767378632054502867494001358512080462196169713300929045961498830402519368250482261411051870211262242497829333777383948056280563964649530350408669661433461803150446009136351281452896391608584617613418765681054343082371159889064236820704531515393806041951619213508773009263079059796970116044846588039909728727712792743200832981991934947545227424367417553196008045019273213153407141325860992204181113914242589817284292567010182844484073025124540318250217631397741251171433100279808847448000751815888433156682294951915225669322199689609592203967800688414725438234666324134853206086827246309011917186535171050153775964680791415629539321729481682323776610037833573627909067984149680174965624246397055685520172119140625");
    freeBigInt(c);
})

DEFINE_TEST(testMulBigInt13, "-", {
    BigInt* a = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    BigInt* n = createBigIntFrom(1);
    for (size_t i = 0; i < 8; i++) {
        BigInt* nn = mulBigInt(n, a);
        freeBigInt(n);
        n = nn;
    }
    freeBigInt(a);
    BigInt* b = createBigIntFromString(str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    BigInt* m = createBigIntFrom(1);
    for (size_t i = 0; i < 20; i++) {
        BigInt* mm = mulBigInt(m, b);
        freeBigInt(m);
        m = mm;
    }
    freeBigInt(b);
    BigInt* c = mulBigInt(n, m);
    ASSERT_BIGINT_STR(c, "374617007032687418676799751535214928784578940190670333347949034709791353629369389765739884075385237457774135079777306232632207674690051324460677006586592604676689578082150100019969948241960827245696748706074783128409459285224344797027501528979129000133549492427250845664470657736691726372566743045360204916823806056463147599675341992660213179135389733714249781331875790459441250727146562697408331103658723341721946911456942654682854902730424565093390849970970944722178820006067106544725713224408683554095085886315883708451137801278814473273675846591482172948453774854244808116798802587865500255137636020850773508157594391718544290742648966797011475841556174959659170692597403524543288368420713596709320985763139934616242223513545677901219236690673388366634457945738884959558088825547180034824472905659896516984720748458003986010260030121291410197708015198626348077505797409466458701043416065226973548463712340399850320706317411381716432275184787701877971135027339052039180868439621708801911179708633503454000149126352823050775892560002562153657431059773025415979640053097629778262289628401049412786960601806640625");
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

