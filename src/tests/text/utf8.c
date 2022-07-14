
#include <unistd.h>

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

DEFINE_TEST(testEncodeUtf8Ascii, "-", {
    char str[] = "_____";
    size_t len = encodeUTF8(0x61, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_STR_EQUAL(str, "a____");
})

DEFINE_TEST(testEncodeUtf8TwoBytes, "-", {
    char str[] = "_____";
    size_t len = encodeUTF8(0xe4, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_STR_EQUAL(str, "ä___");
})

DEFINE_TEST(testEncodeUtf8ThreeBytes, "-", {
    char str[] = "_____";
    size_t len = encodeUTF8(0x2192, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_STR_EQUAL(str, "→__");
})

DEFINE_TEST(testEncodeUtf8FourBytes, "-", {
    char str[] = "_____";
    size_t len = encodeUTF8(0x1F83A, str, strlen(str));
    ASSERT_EQUAL(len, 4);
    ASSERT_STR_EQUAL(str, "🠺_");
})

DEFINE_TEST(testEncodeUtf8AsciiExact, "-", {
    char str[] = "_";
    size_t len = encodeUTF8(0x61, str, strlen(str));
    ASSERT_EQUAL(len, 1);
    ASSERT_STR_EQUAL(str, "a");
})

DEFINE_TEST(testEncodeUtf8TwoBytesExact, "-", {
    char str[] = "__";
    size_t len = encodeUTF8(0xe4, str, strlen(str));
    ASSERT_EQUAL(len, 2);
    ASSERT_STR_EQUAL(str, "ä");
})

DEFINE_TEST(testEncodeUtf8ThreeBytesExact, "-", {
    char str[] = "___";
    size_t len = encodeUTF8(0x2192, str, strlen(str));
    ASSERT_EQUAL(len, 3);
    ASSERT_STR_EQUAL(str, "→");
})

DEFINE_TEST(testEncodeUtf8FourBytesExact, "-", {
    char str[] = "____";
    size_t len = encodeUTF8(0x1F83A, str, strlen(str));
    ASSERT_EQUAL(len, 4);
    ASSERT_STR_EQUAL(str, "🠺");
})

DEFINE_TEST(testEncodeUtf8AsciiNoSpace, "-", {
    char str[] = "";
    size_t len = encodeUTF8(0x61, str, strlen(str));
    ASSERT_EQUAL(len, 0);
    ASSERT_STR_EQUAL(str, "");
})

DEFINE_TEST(testEncodeUtf8TwoBytesNoSpace, "-", {
    char str[] = "_";
    size_t len = encodeUTF8(0xe4, str, strlen(str));
    ASSERT_EQUAL(len, 0);
    ASSERT_STR_EQUAL(str, "_");
})

DEFINE_TEST(testEncodeUtf8ThreeBytesNoSpace, "-", {
    char str[] = "__";
    size_t len = encodeUTF8(0x2192, str, strlen(str));
    ASSERT_EQUAL(len, 0);
    ASSERT_STR_EQUAL(str, "__");
})

DEFINE_TEST(testEncodeUtf8FourBytesNoSpace, "-", {
    char str[] = "___";
    size_t len = encodeUTF8(0x1F83A, str, strlen(str));
    ASSERT_EQUAL(len, 0);
    ASSERT_STR_EQUAL(str, "___");
})

DEFINE_TEST(testUtf8StreamNoop, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("aä→🠺_"));
})

DEFINE_TEST(testUtf8StreamPeekAscii, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("aä→🠺_"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x61);
})

DEFINE_TEST(testUtf8StreamPeekTwoBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("ä→🠺_"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0xe4);
})

DEFINE_TEST(testUtf8StreamPeekThreeBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("→🠺_"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x2192);
})

DEFINE_TEST(testUtf8StreamPeekFourBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("🠺_"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x1F83A);
})

DEFINE_TEST(testUtf8StreamPeekAsciiInvalid, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str(""));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekTwoBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xc3__"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekThreeBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xe2\x86__"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekFourBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xf0\x9f\xa0__"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekTwoBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xc3"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekThreeBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xe2\x86"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamPeekFourBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xf0\x9f\xa0"));
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testUtf8StreamNextAscii, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("aä→🠺_"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), 0x61);
    ASSERT_EQUAL(stream.offset, 1);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0xe4);
})

DEFINE_TEST(testUtf8StreamNextTwoBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("ä→🠺_"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), 0xe4);
    ASSERT_EQUAL(stream.offset, 2);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x2192);
})

DEFINE_TEST(testUtf8StreamNextThreeBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("→🠺_"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), 0x2192);
    ASSERT_EQUAL(stream.offset, 3);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x1F83A);
})

DEFINE_TEST(testUtf8StreamNextFourBytes, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("🠺_"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), 0x1F83A);
    ASSERT_EQUAL(stream.offset, 4);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), '_');
})

DEFINE_TEST(testUtf8StreamNextAsciiInvalid, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str(""));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 0);
})

DEFINE_TEST(testUtf8StreamNextTwoBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xc3__"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 1);
})

DEFINE_TEST(testUtf8StreamNextThreeBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xe2\x86__"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 2);
})

DEFINE_TEST(testUtf8StreamNextFourBytesInvalid1, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xf0\x9f\xa0__"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 3);
})

DEFINE_TEST(testUtf8StreamNextTwoBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xc3"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 1);
})

DEFINE_TEST(testUtf8StreamNextThreeBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xe2\x86"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 2);
})

DEFINE_TEST(testUtf8StreamNextFourBytesInvalid2, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("\xf0\x9f\xa0"));
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    ASSERT_EQUAL(stream.offset, 3);
})

DEFINE_TEST(testUtf8StreamPositionUtf8Stream, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("aä→🠺_"));
    positionUtf8Stream(&stream, 11);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), INVALID_CODEPOINT);
    positionUtf8Stream(&stream, 10);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), '_');
    positionUtf8Stream(&stream, 6);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x1F83A);
    positionUtf8Stream(&stream, 3);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x2192);
    positionUtf8Stream(&stream, 1);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0xe4);
    positionUtf8Stream(&stream, 0);
    ASSERT_EQUAL(peekUtf8CodePoint(&stream), 0x61);
})

DEFINE_TEST(testUtf8StreamPositionUtf8StreamOutsideRange, "-", {
    Utf8Stream stream;
    initUtf8Stream(&stream, str("aä→🠺_"));
    positionUtf8Stream(&stream, 100);
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
    positionUtf8Stream(&stream, -100);
    ASSERT_EQUAL(nextUtf8CodePoint(&stream), INVALID_CODEPOINT);
})

DEFINE_TEST(testReadUtf8FromFileStream, "-", {
    int fds[2];
    pipe(fds);
    FILE* file_r = fdopen(fds[0], "r");
    FILE* file_w = fdopen(fds[1], "w");
    fputs("aä→🠺_", file_w);
    fclose(file_w);
    CodePoint code;
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, 0x61);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 2);
    ASSERT_EQUAL(code, 0xe4);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 3);
    ASSERT_EQUAL(code, 0x2192);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 4);
    ASSERT_EQUAL(code, 0x1F83A);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, '_');
    fclose(file_r);
})

DEFINE_TEST(testReadUtf8FromFileStreamInvalid1, "-", {
    int fds[2];
    pipe(fds);
    FILE* file_r = fdopen(fds[0], "r");
    FILE* file_w = fdopen(fds[1], "w");
    fclose(file_w);
    CodePoint code;
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 0);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
    fclose(file_r);
})

DEFINE_TEST(testReadUtf8FromFileStreamInvalid2, "-", {
    int fds[2];
    pipe(fds);
    FILE* file_r = fdopen(fds[0], "r");
    FILE* file_w = fdopen(fds[1], "w");
    fputs("\xc3_", file_w);
    fclose(file_w);
    CodePoint code;
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, '_');
    fclose(file_r);
})

DEFINE_TEST(testReadUtf8FromFileStreamInvalid3, "-", {
    int fds[2];
    pipe(fds);
    FILE* file_r = fdopen(fds[0], "r");
    FILE* file_w = fdopen(fds[1], "w");
    fputs("\xe2\x86_", file_w);
    fclose(file_w);
    CodePoint code;
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 2);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, '_');
    fclose(file_r);
})

DEFINE_TEST(testReadUtf8FromFileStreamInvalid4, "-", {
    int fds[2];
    pipe(fds);
    FILE* file_r = fdopen(fds[0], "r");
    FILE* file_w = fdopen(fds[1], "w");
    fputs("\xf0\x9f\xa0_", file_w);
    fclose(file_w);
    CodePoint code;
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 3);
    ASSERT_EQUAL(code, INVALID_CODEPOINT);
    ASSERT_EQUAL(readUtf8FromFileStream(file_r, &code), 1);
    ASSERT_EQUAL(code, '_');
    fclose(file_r);
})

DEFINE_TEST(testGetCodePointWidthZeroWidth, "-", {
    ASSERT_EQUAL(getCodePointWidth('\n'), 0);
    ASSERT_EQUAL(getCodePointWidth('\0'), 0);
    ASSERT_EQUAL(getCodePointWidth('\a'), 0);
    ASSERT_EQUAL(getCodePointWidth(0x300), 0);
})

DEFINE_TEST(testGetCodePointWidthZeroNegOne, "-", {
    ASSERT_EQUAL(getCodePointWidth('\b'), -1);
})

DEFINE_TEST(testGetCodePointWidthZeroOne, "-", {
    ASSERT_EQUAL(getCodePointWidth('\t'), 1);
    ASSERT_EQUAL(getCodePointWidth('0'), 1);
    ASSERT_EQUAL(getCodePointWidth('A'), 1);
    ASSERT_EQUAL(getCodePointWidth('a'), 1);
    ASSERT_EQUAL(getCodePointWidth('_'), 1);
    ASSERT_EQUAL(getCodePointWidth(0xe4), 1);
    ASSERT_EQUAL(getCodePointWidth(0x2192), 1);
    ASSERT_EQUAL(getCodePointWidth(0x1F83A), 1);
})

DEFINE_TEST(testGetCodePointWidthZeroTwo, "-", {
    ASSERT_EQUAL(getCodePointWidth(0x3051), 2);
    ASSERT_EQUAL(getCodePointWidth(0x30E8), 2);
})

DEFINE_TEST(testGetStringWidth, "-", {
    ASSERT_EQUAL(getStringWidth(str("")), 0);
    ASSERT_EQUAL(getStringWidth(str("a")), 1);
    ASSERT_EQUAL(getStringWidth(str("aaaaa")), 5);
    ASSERT_EQUAL(getStringWidth(str("\t\t aa")), 5);
    ASSERT_EQUAL(getStringWidth(str("ä→🠺")), 3);
    ASSERT_EQUAL(getStringWidth(str("ぁぁ")), 4);
    ASSERT_EQUAL(getStringWidth(str("\xcc\x80_")), 1);
})

