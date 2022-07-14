
#include "tests/assert.h"
#include "text/utf8.h"

DEFINE_TEST(testDecodeUtf8Ascii, "-", {
    const char* str = "aaa";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_EQUAL(code, 0x61);
})

DEFINE_TEST(testDecodeUtf8TwoBytes, "-", {
    const char* str = "äaa";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_EQUAL(code, 0xe4);
})

DEFINE_TEST(testDecodeUtf8ThreeBytes, "-", {
    const char* str = "→aa";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_EQUAL(code, 0x2192);
})

DEFINE_TEST(testDecodeUtf8FourBytes, "-", {
    const char* str = "🠺aa";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 4);
    ASSERT_EQUAL(code, 0x1F83A);
})

DEFINE_TEST(testDecodeUtf8AsciiSingle, "-", {
    const char* str = "a";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_EQUAL(code, 0x61);
})

DEFINE_TEST(testDecodeUtf8TwoBytesSingle, "-", {
    const char* str = "ä";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_EQUAL(code, 0xe4);
})

DEFINE_TEST(testDecodeUtf8ThreeBytesSingle, "-", {
    const char* str = "→";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_EQUAL(code, 0x2192);
})

DEFINE_TEST(testDecodeUtf8FourBytesSingle, "-", {
    const char* str = "🠺";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 4);
    ASSERT_EQUAL(code, 0x1F83A);
})

DEFINE_TEST(testDecodeUtf8AsciiTruncated, "-", {
    const char* str = "";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 0);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8TwoBytesTruncated1, "-", {
    const char* str = "\xc3";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8ThreeBytesTruncated1, "-", {
    const char* str = "\xe2\x86";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8FourBytesTruncated1, "-", {
    const char* str = "\xf0\x9f\xa0";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8TwoBytesTruncated2, "-", {
    const char* str = "\xc3__";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8ThreeBytesTruncated2, "-", {
    const char* str = "\xe2\x86__";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

DEFINE_TEST(testDecodeUtf8FourBytesTruncated2, "-", {
    const char* str = "\xf0\x9f\xa0__";
    CodePoint code;
    size_t len = decodeUTF8(&code, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
})

