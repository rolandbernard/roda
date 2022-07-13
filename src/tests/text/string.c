
#include "tests/assert.h"
#include "text/string.h"
#include "util/alloc.h"

DEFINE_TEST(testStringStrMacro, "-", {
    ConstString str = str("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
})

DEFINE_TEST(testStringMutStrMacro, "-", {
    String str = mutstr(strdup("Hello world"));
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    str.data[0] = 'h';
    ASSERT_STR_EQUAL(str.data, "hello world");
    freeString(str);
})

DEFINE_TEST(testStringCStrMacro, "-", {
    String str = copyFromCString("Hello world");
    ASSERT_STR_EQUAL(cstr(str), "Hello world");
    freeString(str);
})

DEFINE_TEST(testStringToCnStrMacro, "-", {
    String str = copyFromCString("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    ConstString const_str = tocnstr(str);
    ASSERT_STR_EQUAL(const_str.data, "Hello world");
    ASSERT_EQUAL(const_str.length, 11);
    freeString(str);
})

DEFINE_TEST(testCreateString, "-", {
    String str = createString(strdup("Hello world"), 11);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testCreateConstString, "-", {
    ConstString str = createConstString("Hello world", 11);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
})

DEFINE_TEST(testCreateFromCString, "-", {
    String str = createFromCString(strdup("Hello world"));
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testCreateFromConstCString, "-", {
    ConstString str = createFromConstCString("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
})

DEFINE_TEST(testCreateEmptyString, "-", {
    String str = createEmptyString();
    ASSERT_STR_EQUAL(str.data, "");
    ASSERT_EQUAL(str.length, 0);
    freeString(str);
})

DEFINE_TEST(testCreateEmptyConstString, "-", {
    ConstString str = createEmptyConstString();
    ASSERT_STR_EQUAL(str.data, "");
    ASSERT_EQUAL(str.length, 0);
})

DEFINE_TEST(testCopyString, "-", {
    ConstString str = str("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    String str_copy = copyString(str);
    ASSERT_UNEQUAL(str.data, str_copy.data);
    ASSERT_STR_EQUAL(str_copy.data, "Hello world");
    ASSERT_EQUAL(str_copy.length, 11);
    freeString(str_copy);
})

DEFINE_TEST(testConcatStrings, "-", {
    ConstString str0 = str("Hello ");
    ConstString str1 = str("world");
    String str = concatStrings(str0, str1);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testConcatNStringsOne, "-", {
    ConstString str0 = str("Hello world");
    String str = concatNStrings(1, str0);
    ASSERT_UNEQUAL(str0.data, str.data);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testConcatNStringsTwo, "-", {
    ConstString str0 = str("Hello ");
    ConstString str1 = str("world");
    String str = concatNStrings(2, str0, str1);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testConcatNStringsMany, "-", {
    ConstString str0 = str("Hel");
    ConstString str1 = str("lo");
    ConstString str2 = str(" ");
    ConstString str3 = str("wo");
    ConstString str4 = str("rld");
    String str = concatNStrings(5, str0, str1, str2, str3, str4);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testToConstString, "-", {
    String str0 = copyFromCString("Hello world");
    ConstString str = toConstString(str0);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str0);
})

DEFINE_TEST(testToNonConstString, "-", {
    // Only save if it was previously not const
    String str0 = copyFromCString("Hello world");
    ConstString str1 = toConstString(str0);
    String str = toNonConstString(str1);
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    str.data[0] = 'h';
    ASSERT_STR_EQUAL(str.data, "hello world");
    freeString(str);
})

DEFINE_TEST(testCompareStringsEqual, "-", {
    ConstString str0 = str("aa");
    ConstString str1 = str("aa");
    ASSERT_TRUE(compareStrings(str0, str1) == 0);
})

DEFINE_TEST(testCompareStringsLess0, "-", {
    ConstString str0 = str("aa");
    ConstString str1 = str("ba");
    ASSERT_TRUE(compareStrings(str0, str1) == -1);
})

DEFINE_TEST(testCompareStringsLess1, "-", {
    ConstString str0 = str("aa");
    ConstString str1 = str("ab");
    ASSERT_TRUE(compareStrings(str0, str1) == -1);
})

DEFINE_TEST(testCompareStringsMore0, "-", {
    ConstString str0 = str("ba");
    ConstString str1 = str("aa");
    ASSERT_TRUE(compareStrings(str0, str1) == 1);
})

DEFINE_TEST(testCompareStringsMore1, "-", {
    ConstString str0 = str("ab");
    ConstString str1 = str("aa");
    ASSERT_TRUE(compareStrings(str0, str1) == 1);
})

DEFINE_TEST(testToCString, "-", {
    String str = copyFromCString("Hello world");
    ASSERT_STR_EQUAL(cstr(str), "Hello world");
    freeString(str);
})

DEFINE_TEST(testCopyToCString, "-", {
    ConstString str = str("Hello world");
    char* cstr = copyToCString(str);
    ASSERT_STR_EQUAL(cstr, "Hello world");
    ASSERT_UNEQUAL(cstr, str.data);
    cstr[0] = 'h';
    ASSERT_STR_EQUAL(cstr, "hello world");
    FREE(cstr);
})

DEFINE_TEST(testResizeStringData, "-", {
    String str = copyFromCString("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    str.length = 5;
    str = resizeStringData(str);
    ASSERT_STR_EQUAL(str.data, "Hello");
    freeString(str);
})

DEFINE_TEST(testFindFirstIndexOfCharFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findFirstIndexOfChar(str, 'H'), 0);
    ASSERT_EQUAL(findFirstIndexOfChar(str, 'l'), 2);
    ASSERT_EQUAL(findFirstIndexOfChar(str, 'w'), 6);
    ASSERT_EQUAL(findFirstIndexOfChar(str, 'd'), 10);
})

DEFINE_TEST(testFindFirstIndexOfCharNotFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findFirstIndexOfChar(str, 'A'), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfChar(str, '-'), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfChar(str, '_'), NO_POS);
})

DEFINE_TEST(testFindFirstIndexOfStringFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findFirstIndexOfString(str, str("")), 0);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("H")), 0);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("He")), 0);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("Hello")), 0);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("Hello world")), 0);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("ld")), 9);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("d")), 10);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("l")), 2);
})

DEFINE_TEST(testFindFirstIndexOfStringNotFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findFirstIndexOfString(str, str("Hello world_")), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("Hallo")), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("lld")), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("dd")), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("-")), NO_POS);
    ASSERT_EQUAL(findFirstIndexOfString(str, str("__")), NO_POS);
})

DEFINE_TEST(testFindLastIndexOfCharFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findLastIndexOfChar(str, 'H'), 0);
    ASSERT_EQUAL(findLastIndexOfChar(str, 'w'), 6);
    ASSERT_EQUAL(findLastIndexOfChar(str, 'l'), 9);
    ASSERT_EQUAL(findLastIndexOfChar(str, 'd'), 10);
})

DEFINE_TEST(testFindLastIndexOfCharNotFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findLastIndexOfChar(str, 'A'), NO_POS);
    ASSERT_EQUAL(findLastIndexOfChar(str, '-'), NO_POS);
    ASSERT_EQUAL(findLastIndexOfChar(str, '_'), NO_POS);
})

DEFINE_TEST(testFindLastIndexOfStringFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findLastIndexOfString(str, str("")), 11);
    ASSERT_EQUAL(findLastIndexOfString(str, str("H")), 0);
    ASSERT_EQUAL(findLastIndexOfString(str, str("He")), 0);
    ASSERT_EQUAL(findLastIndexOfString(str, str("Hello")), 0);
    ASSERT_EQUAL(findLastIndexOfString(str, str("Hello world")), 0);
    ASSERT_EQUAL(findLastIndexOfString(str, str("ld")), 9);
    ASSERT_EQUAL(findLastIndexOfString(str, str("d")), 10);
    ASSERT_EQUAL(findLastIndexOfString(str, str("l")), 9);
    ASSERT_EQUAL(findLastIndexOfString(str, str("d")), 10);
    ASSERT_EQUAL(findLastIndexOfString(str, str("ld")), 9);
    ASSERT_EQUAL(findLastIndexOfString(str, str("rld")), 8);
    ASSERT_EQUAL(findLastIndexOfString(str, str("world")), 6);
})

DEFINE_TEST(testFindLastIndexOfStringNotFound, "-", {
    ConstString str = str("Hello world");
    ASSERT_EQUAL(findLastIndexOfString(str, str("Hello world_")), NO_POS);
    ASSERT_EQUAL(findLastIndexOfString(str, str("Hallo")), NO_POS);
    ASSERT_EQUAL(findLastIndexOfString(str, str("lld")), NO_POS);
    ASSERT_EQUAL(findLastIndexOfString(str, str("dd")), NO_POS);
    ASSERT_EQUAL(findLastIndexOfString(str, str("-")), NO_POS);
    ASSERT_EQUAL(findLastIndexOfString(str, str("__")), NO_POS);
})

DEFINE_TEST(testGetStringBeforeChar, "-", {
    ConstString str = str("Hello world");
    char* cstr = copyToCString(getStringBeforeChar(str, 'H'));
    ASSERT_STR_EQUAL(cstr, "");
    FREE(cstr);
    cstr = copyToCString(getStringBeforeChar(str, 'l'));
    ASSERT_STR_EQUAL(cstr, "He");
    FREE(cstr);
    cstr = copyToCString(getStringBeforeChar(str, 'w'));
    ASSERT_STR_EQUAL(cstr, "Hello ");
    FREE(cstr);
    cstr = copyToCString(getStringBeforeChar(str, 'd'));
    ASSERT_STR_EQUAL(cstr, "Hello worl");
    FREE(cstr);
    cstr = copyToCString(getStringAfterChar(str, '-'));
    ASSERT_STR_EQUAL(cstr, "Hello world");
    FREE(cstr);
})

DEFINE_TEST(testGetStringAfterChar, "-", {
    ConstString str = str("Hello world");
    ASSERT_STR_EQUAL(toCString(getStringAfterChar(str, 'H')), "ello world");
    ASSERT_STR_EQUAL(toCString(getStringAfterChar(str, 'l')), "d");
    ASSERT_STR_EQUAL(toCString(getStringAfterChar(str, 'w')), "orld");
    ASSERT_STR_EQUAL(toCString(getStringAfterChar(str, '-')), "Hello world");
})

DEFINE_TEST(testStringBuilderSimple, "-", {
    StringBuilder builder;
    initStringBuilder(&builder);
    deinitStringBuilder(&builder);
})

DEFINE_TEST(testStringBuilderEmpty, "-", {
    StringBuilder builder;
    initStringBuilder(&builder);
    String str = builderToString(&builder);
    ASSERT_STR_EQUAL(cstr(str), "");
    ASSERT_EQUAL(str.length, 0);
    freeString(str);
})

DEFINE_TEST(testStringBuilderPushString, "-", {
    StringBuilder builder;
    initStringBuilder(&builder);
    pushToStringBuilder(&builder, str("Hello"));
    pushToStringBuilder(&builder, str(" "));
    pushToStringBuilder(&builder, str("world"));
    String str = builderToString(&builder);
    ASSERT_STR_EQUAL(cstr(str), "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

