
#include "tests/assert.h"
#include "const/fixedint.h"

#define ASSERT_FIXEDINT_STR(INT, STR) {             \
    String str = stringForFixedIntSigned(INT, 10);  \
    ASSERT_STR_EQUAL(cstr(str), STR);               \
    freeString(str);                                \
}

#define ASSERT_FIXEDINT(INT, NUM) \
    ASSERT_EQUAL(intMaxForFixedInt(INT), NUM);

DEFINE_TEST(testCreateFixedInt, "-", {
    FixedInt* a = createFixedInt(128);
    ASSERT_FIXEDINT(a, 0);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromSmall, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    ASSERT_FIXEDINT(a, 42);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromLarge, "-", {
    FixedInt* a = createFixedIntFrom(128, 1234567890123456789);
    ASSERT_FIXEDINT(a, 1234567890123456789);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromSmallNeg, "-", {
    FixedInt* a = createFixedIntFrom(128, -123);
    ASSERT_FIXEDINT(a, -123);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromLargeNeg, "-", {
    FixedInt* a = createFixedIntFrom(128, -98765432100000000);
    ASSERT_FIXEDINT(a, -98765432100000000);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringSmall, "-", {
    FixedInt* a = createFixedIntFromString(10, str("42"), 10);
    ASSERT_FIXEDINT(a, 42);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringLarge, "-", {
    FixedInt* a = createFixedIntFromString(128, str("1234567890123456789"), 10);
    ASSERT_FIXEDINT(a, 1234567890123456789);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringSmallNeg, "-", {
    FixedInt* a = createFixedIntFromString(10, str("-123"), 10);
    ASSERT_FIXEDINT(a, -123);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringLargeNeg, "-", {
    FixedInt* a = createFixedIntFromString(60, str("-98765432100000000"), 10);
    ASSERT_FIXEDINT(a, -98765432100000000);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringHex, "-", {
    FixedInt* a = createFixedIntFromString(78, str("fa03"), 16);
    ASSERT_FIXEDINT(a, 64003);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringBin, "-", {
    FixedInt* a = createFixedIntFromString(128, str("1000101111010011"), 2);
    ASSERT_FIXEDINT(a, 35795);
    freeFixedInt(a);
})

DEFINE_TEST(testCreateFixedIntFromStringLarge2, "-", {
    FixedInt* a = createFixedIntFromString(256, str("123456789abcdef00000000000000000000000000"), 16);
    ASSERT_FIXEDINT_STR(a, "1662864085140938409653700456423626137158360760320");
    freeFixedInt(a);
})

DEFINE_TEST(testCopyFixedIntZero, "-", {
    FixedInt* a = createFixedInt(128);
    FixedInt* b = copyFixedInt(a);
    freeFixedInt(a);
    ASSERT_FIXEDINT(b, 0);
    freeFixedInt(b);
})

DEFINE_TEST(testCopyFixedIntFromSmall, "-", {
    FixedInt* a = createFixedIntFrom(36, 42);
    FixedInt* b = copyFixedInt(a);
    freeFixedInt(a);
    ASSERT_FIXEDINT(b, 42);
    freeFixedInt(b);
})

DEFINE_TEST(testCopyFixedIntFromLarge, "-", {
    FixedInt* a = createFixedIntFrom(512, 1234567890123456789);
    FixedInt* b = copyFixedInt(a);
    freeFixedInt(a);
    ASSERT_FIXEDINT(b, 1234567890123456789);
    freeFixedInt(b);
})

DEFINE_TEST(testCopyFixedIntFromSmallNeg, "-", {
    FixedInt* a = createFixedIntFrom(12, -123);
    FixedInt* b = copyFixedInt(a);
    freeFixedInt(a);
    ASSERT_FIXEDINT(b, -123);
    freeFixedInt(b);
})

DEFINE_TEST(testCopyFixedIntFromLargeNeg, "-", {
    FixedInt* a = createFixedIntFrom(68, -98765432100000000);
    FixedInt* b = copyFixedInt(a);
    freeFixedInt(a);
    ASSERT_FIXEDINT(b, -98765432100000000);
    freeFixedInt(b);
})

DEFINE_TEST(testSignOfFixedIntZero, "-", {
    FixedInt* a = createFixedInt(22);
    ASSERT_EQUAL(signOfFixedInt(a), 0);
    freeFixedInt(a);
})

DEFINE_TEST(testSignOfFixedIntPos, "-", {
    FixedInt* a = createFixedIntFrom(33, 42);
    ASSERT_EQUAL(signOfFixedInt(a), 1);
    freeFixedInt(a);
})

DEFINE_TEST(testSignOfFixedIntNeg, "-", {
    FixedInt* a = createFixedIntFrom(88, -123456789);
    ASSERT_EQUAL(signOfFixedInt(a), -1);
    freeFixedInt(a);
})

DEFINE_TEST(testIsZero, "-", {
    FixedInt* a = createFixedInt(78);
    ASSERT_TRUE(isFixedIntZero(a));
    freeFixedInt(a);
})

DEFINE_TEST(testIsZeroPos, "-", {
    FixedInt* a = createFixedIntFrom(99, 42);
    ASSERT_FALSE(isFixedIntZero(a));
    freeFixedInt(a);
})

DEFINE_TEST(testIsZeroNeg, "-", {
    FixedInt* a = createFixedIntFrom(128, -123456789);
    ASSERT_FALSE(isFixedIntZero(a));
    freeFixedInt(a);
})

DEFINE_TEST(testCompareFixedIntEqual1, "-", {
    FixedInt* a = createFixedIntFrom(32, 42);
    FixedInt* b = createFixedIntFrom(32, 42);
    ASSERT_EQUAL(compareFixedIntUnsigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntEqual2, "-", {
    FixedInt* a = createFixedIntFrom(32, -42);
    FixedInt* b = createFixedIntFrom(32, -42);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntEqual3, "-", {
    FixedInt* a = createFixedIntFromString(256, str("0123456789abcdefghijklmnopqrstuvwxyz"), 36);
    FixedInt* b = createFixedIntFromString(256, str("123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 36);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntLess1, "-", {
    FixedInt* a = createFixedIntFrom(32, 41);
    FixedInt* b = createFixedIntFrom(32, 42);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), -1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntLess2, "-", {
    FixedInt* a = createFixedIntFrom(16, -42);
    FixedInt* b = createFixedIntFrom(16, 41);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), -1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntLess3, "-", {
    FixedInt* a = createFixedIntFromString(512, str("-0123456789abcdefghijklmnopqrstuvwxyz"), 36);
    FixedInt* b = createFixedIntFromString(512, str("-123456789ABCDEFGHIJKLMNOPQRSTUVWXY"), 36);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), -1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntMore1, "-", {
    FixedInt* a = createFixedIntFrom(8, 43);
    FixedInt* b = createFixedIntFrom(8, 42);
    ASSERT_EQUAL(compareFixedIntUnsigned(a, b), 1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntMore2, "-", {
    FixedInt* a = createFixedIntFrom(32, 42);
    FixedInt* b = createFixedIntFrom(32, -43);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testCompareFixedIntMore3, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("-0123456789abcdefghijklmnopqrstuvwxy"), 36);
    FixedInt* b = createFixedIntFromString(1024, str("-123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 36);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 1);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testNegFixedIntZero, "-", {
    FixedInt* a = createFixedInt(32);
    FixedInt* b = negFixedInt(a);
    ASSERT_FIXEDINT(b, 0);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testNegFixedIntPos, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = negFixedInt(a);
    ASSERT_FIXEDINT(b, -42);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testNegFixedIntNeg, "-", {
    FixedInt* a = createFixedIntFrom(8, -42);
    FixedInt* b = negFixedInt(a);
    ASSERT_FIXEDINT(b, 42);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testAbsFixedIntZero, "-", {
    FixedInt* a = createFixedInt(16);
    FixedInt* b = absFixedInt(a);
    ASSERT_FIXEDINT(b, 0);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testAbsFixedIntPos, "-", {
    FixedInt* a = createFixedIntFrom(32, 42);
    FixedInt* b = absFixedInt(a);
    ASSERT_FIXEDINT(b, 42);
    ASSERT_EQUAL(compareFixedIntSigned(a, b), 0);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testAbsFixedIntNeg, "-", {
    FixedInt* a = createFixedIntFrom(32, -42);
    FixedInt* b = absFixedInt(a);
    ASSERT_FIXEDINT(b, 42);
    freeFixedInt(a);
    freeFixedInt(b);
})

DEFINE_TEST(testAddFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(64, 42);
    FixedInt* b = createFixedIntFrom(64, 42);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT(c, 84);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, 9145678912345678901);
    FixedInt* b = createFixedIntFrom(128, 9145678912345678901);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "18291357824691357802");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(32, -42);
    FixedInt* b = createFixedIntFrom(32, 20);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT(c, -22);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(32, 42);
    FixedInt* b = createFixedIntFrom(32, -20);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT(c, 22);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt5, "-", {
    FixedInt* a = createFixedIntFrom(32, -42);
    FixedInt* b = createFixedIntFrom(32, -20);
    FixedInt* c = addFixedInt(a, b);
    fprintf(stderr, "%li\n", intMaxForFixedInt(c));
    ASSERT_FIXEDINT(c, -62);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt6, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "14966048989831189770931389386893450031259204");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt7, "-", {
    FixedInt* a = createFixedIntFromString(2048, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(2048, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = addFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "14966505724369675501907842622245151975259546");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAddFixedInt8, "-", {
    FixedInt* a = createFixedIntFrom(512, 4294967295);
    FixedInt* c = addFixedInt(a, a);
    ASSERT_FIXEDINT_STR(c, "8589934590");
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(60, 42);
    FixedInt* b = createFixedIntFrom(60, 42);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT(c, 0);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(38, 42);
    FixedInt* b = createFixedIntFrom(38, -42);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT(c, 84);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(40, -42);
    FixedInt* b = createFixedIntFrom(40, -20);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT(c, -22);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(32, -42);
    FixedInt* b = createFixedIntFrom(32, 20);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT(c, -62);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt5, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "14966048989831189770931389386893450031259204");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testSubFixedInt6, "-", {
    FixedInt* a = createFixedIntFromString(2048, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(2048, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = subFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "14966505724369675501907842622245151975259546");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(32, 42);
    FixedInt* b = createFixedIntFrom(32, 20);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT(c, 840);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(50, 42);
    FixedInt* b = createFixedIntFrom(50, -987654321);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT(c, -41481481482);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(99, -123456789);
    FixedInt* b = createFixedIntFrom(99, 20);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT(c, -2469135780);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(1111, -123456789123456789);
    FixedInt* b = createFixedIntFrom(1111, -987654321987654321);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "121932631356500531347203169112635269");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt5, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt6, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "-3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt7, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "-3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt8, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT_STR(c, "3417807890772355817164787473054487168320873578264084521151908534580112334057353125");
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt9, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* n = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 10; i++) {
        FixedInt* nn = mulFixedInt(n, a);
        freeFixedInt(n);
        n = nn;
    }
    freeFixedInt(a);
    FixedInt* b = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* m = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 10; i++) {
        FixedInt* mm = mulFixedInt(m, b);
        freeFixedInt(m);
        m = mm;
    }
    freeFixedInt(b);
    FixedInt* c = mulFixedInt(n, m);
    ASSERT_FIXEDINT_STR(c, "217508624057813131549155851881461695469597637317736912272709573102411904067361947208499227536292620333034386882841303431942718249874095538876607760187803472373721875699710595347873670453620114942222822331878647291624265481684197376075898076064589999622470238948936453249410166081685720560924493765751339090838692237180175263239536023501057606433389884015132665735879562618010507233775990020352354762637193776340845419203939580640080136501220980571245537852949336540608441848421140647089975112454398901006530160609741692902018243997107594896273809154025070992802543691180331186227338900753582675053722334316140965712566678901919781350796811737638393868126897786329305201094087921428545943301387275379398085994559123917799825686020488311384460944866347571296752002251411776273242364343474264387623406946659088134765625");
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt10, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(6400, str("0"), 16);
    FixedInt* c = mulFixedInt(a, b);
    ASSERT_FIXEDINT(c, 0);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt11, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("0"), 16);
    FixedInt* c = mulFixedInt(b, a);
    ASSERT_FIXEDINT(c, 0);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt12, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* n = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 10; i++) {
        FixedInt* nn = mulFixedInt(n, a);
        freeFixedInt(n);
        n = nn;
    }
    freeFixedInt(a);
    FixedInt* b = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* m = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 20; i++) {
        FixedInt* mm = mulFixedInt(m, b);
        freeFixedInt(m);
        m = mm;
    }
    freeFixedInt(b);
    FixedInt* c = mulFixedInt(n, m);
    ASSERT_FIXEDINT_STR(c, "83910260336482200587328407316528466200710664827330427597248899706087058343113104783347684954300253413692447983297891898659794240959285105469599304016445391044739088751579200140172982077171724978024245150131787193715173793064185274454394241030903827011018862311494923168459444634394864887150786907085955579179212866037414481046286992500260667310401796624893809656799866131100746597430944460877177315771903165575842220314260626251553587017167702065532018955969047918191871027256830170484457767378632054502867494001358512080462196169713300929045961498830402519368250482261411051870211262242497829333777383948056280563964649530350408669661433461803150446009136351281452896391608584617613418765681054343082371159889064236820704531515393806041951619213508773009263079059796970116044846588039909728727712792743200832981991934947545227424367417553196008045019273213153407141325860992204181113914242589817284292567010182844484073025124540318250217631397741251171433100279808847448000751815888433156682294951915225669322199689609592203967800688414725438234666324134853206086827246309011917186535171050153775964680791415629539321729481682323776610037833573627909067984149680174965624246397055685520172119140625");
    freeFixedInt(c);
})

DEFINE_TEST(testMulFixedInt13, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* n = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 8; i++) {
        FixedInt* nn = mulFixedInt(n, a);
        freeFixedInt(n);
        n = nn;
    }
    freeFixedInt(a);
    FixedInt* b = createFixedIntFromString(6400, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* m = createFixedIntFrom(6400, 1);
    for (size_t i = 0; i < 20; i++) {
        FixedInt* mm = mulFixedInt(m, b);
        freeFixedInt(m);
        m = mm;
    }
    freeFixedInt(b);
    FixedInt* c = mulFixedInt(n, m);
    ASSERT_FIXEDINT_STR(c, "374617007032687418676799751535214928784578940190670333347949034709791353629369389765739884075385237457774135079777306232632207674690051324460677006586592604676689578082150100019969948241960827245696748706074783128409459285224344797027501528979129000133549492427250845664470657736691726372566743045360204916823806056463147599675341992660213179135389733714249781331875790459441250727146562697408331103658723341721946911456942654682854902730424565093390849970970944722178820006067106544725713224408683554095085886315883708451137801278814473273675846591482172948453774854244808116798802587865500255137636020850773508157594391718544290742648966797011475841556174959659170692597403524543288368420713596709320985763139934616242223513545677901219236690673388366634457945738884959558088825547180034824472905659896516984720748458003986010260030121291410197708015198626348077505797409466458701043416065226973548463712340399850320706317411381716432275184787701877971135027339052039180868439621708801911179708633503454000149126352823050775892560002562153657431059773025415979640053097629778262289628401049412786960601806640625");
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, 20);
    FixedInt* c = udivFixedInt(a, b);
    ASSERT_FIXEDINT(c, 2);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, 987654321987654321);
    FixedInt* b = createFixedIntFrom(128, -1234);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, -800368170168277);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(128, -123456789123456789);
    FixedInt* b = createFixedIntFrom(128, 42);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, -2939447360082304);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(128, -123456789987654321);
    FixedInt* b = createFixedIntFrom(128, -54321);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, 2272726753698);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt5, "-", {
    FixedInt* a = createFixedIntFrom(128, -54320);
    FixedInt* b = createFixedIntFrom(128, -54321);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, 0);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt6, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = udivFixedInt(a, b);
    ASSERT_FIXEDINT(c, 65536);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt7, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, -65536);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt8, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, 65536);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testDivFixedInt9, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sdivFixedInt(a, b);
    ASSERT_FIXEDINT(c, -65536);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(77, 42);
    FixedInt* b = createFixedIntFrom(77, 20);
    FixedInt* c = uremFixedInt(a, b);
    ASSERT_FIXEDINT(c, 2);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, 12345678987654321);
    FixedInt* b = createFixedIntFrom(128, -12345);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, 696);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(128, -98765432123456789);
    FixedInt* b = createFixedIntFrom(128, 987654321);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, -23456789);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(128, -987654321);
    FixedInt* b = createFixedIntFrom(128, -987654321123456789);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, -987654321);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt5, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = uremFixedInt(a, b);
    ASSERT_FIXEDINT(c, 52719);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt6, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, -52719);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt7, "-", {
    FixedInt* a = createFixedIntFromString(1024, str("abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(1024, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, 52719);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testRemFixedInt8, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefabcdef"), 16);
    FixedInt* b = createFixedIntFromString(6400, str("-abcdefabcdefabcdefabcdefabcdefab"), 16);
    FixedInt* c = sremFixedInt(a, b);
    ASSERT_FIXEDINT(c, -52719);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAndFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = andFixedInt(a, b);
    ASSERT_FIXEDINT(c, 10);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAndFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = andFixedInt(a, b);
    ASSERT_FIXEDINT(c, 18);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAndFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = andFixedInt(a, b);
    ASSERT_FIXEDINT(c, 34);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testAndFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = andFixedInt(a, b);
    ASSERT_FIXEDINT(c, -58);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testOrFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = orFixedInt(a, b);
    ASSERT_FIXEDINT(c, 58);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testOrFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = orFixedInt(a, b);
    ASSERT_FIXEDINT(c, -34);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testOrFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = orFixedInt(a, b);
    ASSERT_FIXEDINT(c, -18);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testOrFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = orFixedInt(a, b);
    ASSERT_FIXEDINT(c, -10);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testXorFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = xorFixedInt(a, b);
    ASSERT_FIXEDINT(c, 48);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testXorFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, 26);
    FixedInt* c = xorFixedInt(a, b);
    ASSERT_FIXEDINT(c, -52);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testXorFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = xorFixedInt(a, b);
    ASSERT_FIXEDINT(c, -52);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testXorFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* b = createFixedIntFrom(128, -26);
    FixedInt* c = xorFixedInt(a, b);
    ASSERT_FIXEDINT(c, 48);
    freeFixedInt(a);
    freeFixedInt(b);
    freeFixedInt(c);
})

DEFINE_TEST(testNotFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* c = notFixedInt(a);
    ASSERT_FIXEDINT(c, -43);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testNotFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, -42);
    FixedInt* c = notFixedInt(a);
    ASSERT_FIXEDINT(c, 41);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftLeftFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* c = shiftLeftFixedInt(a, 0);
    ASSERT_FIXEDINT(c, 42);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftLeftFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(128, 42);
    FixedInt* c = shiftLeftFixedInt(a, 32);
    ASSERT_FIXEDINT(c, 180388626432);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftLeftFixedInt3, "-", {
    FixedInt* a = createFixedIntFrom(6400, 42);
    FixedInt* c = shiftLeftFixedInt(a, 1234);
    ASSERT_FIXEDINT_STR(c, "12424071433540142420521877220076350813026266859685665674061360696127608733442034818964261164431878107830633205880980151711074477771876033959355142924279319687537208647948638557706820526101026562169644639169603467248951302514030775012441411551731304491930775759074912082229886225695359354289776176171318118053274739978971350726662145988449999470415097463527440874884697161728");
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftLeftFixedInt4, "-", {
    FixedInt* a = createFixedIntFrom(6400, -42);
    FixedInt* c = shiftLeftFixedInt(a, 123);
    ASSERT_FIXEDINT_STR(c, "-446620606583731733295679172254195777536");
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftRightFixedInt1, "-", {
    FixedInt* a = createFixedIntFrom(6400, 12345678987654321);
    FixedInt* c = shiftRightLogicalFixedInt(a, 32);
    ASSERT_FIXEDINT(c, 2874452);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftRightFixedInt2, "-", {
    FixedInt* a = createFixedIntFrom(6400, 12345678987654321);
    FixedInt* c = shiftRightLogicalFixedInt(a, 42);
    ASSERT_FIXEDINT(c, 2807);
    freeFixedInt(a);
    freeFixedInt(c);
})

DEFINE_TEST(testShiftRightFixedInt3, "-", {
    FixedInt* a = createFixedIntFromString(6400, str("12424071433540142420521877220076350813026266859685665674061360696127608733442034818964261164431878107830633205880980151711074477771876033959355142924279319687537208647948638557706820526101026562169644639169603467248951302514030775012441411551731304491930775759074912082229886225695359354289776176171318118053274739978971350726662145988449999470415097463527440874884697161728"), 10);
    FixedInt* c = shiftRightLogicalFixedInt(a, 1234);
    ASSERT_FIXEDINT(c, 42);
    freeFixedInt(a);
    freeFixedInt(c);
})

