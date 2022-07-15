
#include "tests/assert.h"
#include "text/format.h"

DEFINE_TEST(testCreateFormattedStringSimple, "-", {
    String str = createFormattedString("Hello world");
    ASSERT_STR_EQUAL(str.data, "Hello world");
    ASSERT_EQUAL(str.length, 11);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedStringInt, "-", {
    String str = createFormattedString("Hello world %i", 42);
    ASSERT_STR_EQUAL(str.data, "Hello world 42");
    ASSERT_EQUAL(str.length, 14);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedStringMultipleInt, "-", {
    String str = createFormattedString("Hello world %i %i %i", 1, 12, 42);
    ASSERT_STR_EQUAL(str.data, "Hello world 1 12 42");
    ASSERT_EQUAL(str.length, 19);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedStringFloat, "-", {
    String str = createFormattedString("Hello world %f", 4.2);
    ASSERT_STR_EQUAL(str.data, "Hello world 4.200000");
    ASSERT_EQUAL(str.length, 20);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedStringMultipleFloat, "-", {
    String str = createFormattedString("Hello world %g %f %g", 1.0, 12.0, 4e-20);
    ASSERT_STR_EQUAL(str.data, "Hello world 1 12.000000 4e-20");
    ASSERT_EQUAL(str.length, 29);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedString, "-", {
    String str = createFormattedString("Hello world %s", "Test");
    ASSERT_STR_EQUAL(str.data, "Hello world Test");
    ASSERT_EQUAL(str.length, 16);
    freeString(str);
})

DEFINE_TEST(testCreateFormattedStringMultipleString, "-", {
    String str = createFormattedString("Hello world %s %s %s", "a", "BB", "cCC");
    ASSERT_STR_EQUAL(str.data, "Hello world a BB cCC");
    ASSERT_EQUAL(str.length, 20);
    freeString(str);
})

DEFINE_TEST(testPushFormattedString, "-", {
    StringBuilder builder;
    initStringBuilder(&builder);
    pushFormattedString(&builder, "Hello %i %f %s", 1, 2.0, "_3_");
    String str = builderToString(&builder);
    ASSERT_STR_EQUAL(str.data, "Hello 1 2.000000 _3_");
    ASSERT_EQUAL(str.length, 20);
    freeString(str);
})

