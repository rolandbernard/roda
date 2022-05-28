
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "text/string.h"
#include "text/utf8.h"

#include "util/alloc.h"

size_t findFirstIndexOfChar(ConstString str, char c) {
    for (size_t i = 0; i < str.length; i++) {
        if (str.data[i] == c) {
            return i;
        }
    }
    return NO_POS;
}

size_t findFirstIndexOfString(ConstString str, ConstString pat) {
    for (size_t i = 0; i < str.length - pat.length; i++) {
        bool equal = true;
        for (size_t j = 0; j < pat.length; j++) {
            if (str.data[i + j] != pat.data[j]) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return i;
        }
    }
    return NO_POS;
}

size_t findLastIndexOfChar(ConstString str, char c) {
    for (size_t i = str.length - 1; i != NO_POS; i--) {
        if (str.data[i] == c) {
            return i;
        }
    }
    return NO_POS;
}

size_t findLastIndexOfString(ConstString str, ConstString pat) {
    for (size_t i = str.length - pat.length; i != NO_POS; i--) {
        bool equal = true;
        for (size_t j = 0; j < pat.length; j++) {
            if (str.data[i + j] != pat.data[j]) {
                equal = false;
                break;
            }
        }
        if (equal) {
            return i;
        }
    }
    return NO_POS;
}

ConstString getStringBeforeChar(ConstString string, char c) {
    ConstString ret = {
        .length = findFirstIndexOfChar(string, c) + 1,
        .data = string.data,
    };
    return ret;
}

ConstString getStringAfterChar(ConstString string, char c) {
    ConstString ret = {
        .length = string.length - findLastIndexOfChar(string, c) - 1,
    };
    ret.data = string.data + string.length - ret.length;
    return ret;
}

String createString(char* data, size_t length) {
    String ret = {
        .data = data,
        .length = length,
    };
    return ret;
}

ConstString createConstString(const char* data, size_t length) {
    ConstString ret = {
        .data = data,
        .length = length,
    };
    return ret;
}

String createFromCString(char* data) {
    return createString(data, strlen(data));
}

ConstString createFromConstCString(const char* data) {
    return createConstString(data, strlen(data));
}

String createEmptyString() {
    return createString(NULL, 0);
}

ConstString createEmptyConstString() {
    return createConstString(NULL, 0);
}

String copyString(ConstString string) {
    String ret = {
        .length = string.length,
    };
    ret.data = ALLOC(char, string.length + 1);
    ret.data[string.length] = 0;
    memcpy(ret.data, string.data, string.length);
    return ret;
}

String concatStrings(ConstString a, ConstString b) {
    String ret = {
        .length = a.length + b.length,
    };
    ret.data = ALLOC(char, ret.length + 1);
    ret.data[ret.length] = 0;
    memcpy(ret.data, a.data, a.length);
    memcpy(ret.data + a.length, b.data, b.length);
    return ret;
}

String concatNStrings(size_t n, ...) {
    String ret = {
        .length = 0,
    };
    va_list strings;
    va_start(strings, n);
    for (size_t i = 0; i < n; i++) {
        ConstString string = va_arg(strings, ConstString);
        ret.length += string.length;
    }
    va_end(strings);
    ret.data = ALLOC(char, ret.length + 1);
    ret.data[ret.length] = 0;
    int offset = 0;
    va_start(strings, n);
    for (size_t i = 0; i < n; i++) {
        ConstString string = va_arg(strings, ConstString);
        memcpy(ret.data + offset, string.data, string.length);
        offset += string.length;
    }
    va_end(strings);
    return ret;
}

String cloneCString(const char* cstr) {
    String ret = {
        .length = strlen(cstr),
    };
    ret.data = ALLOC(char, ret.length + 1);
    ret.data[ret.length] = 0;
    memcpy(ret.data, cstr, ret.length);
    return ret;
}

void freeString(String string) {
    FREE(string.data);
}

const char* toCString(ConstString string) {
    return string.data;
}

String toNonConstString(ConstString string) {
    return createString((char*)string.data, string.length);
}

ConstString toConstString(String string) {
    return createConstString((const char*)string.data, string.length);
}

int compareStrings(ConstString a, ConstString b) {
    for (size_t i = 0; i < a.length && i < b.length; i++) {
        if (a.data[i] < b.data[i]) {
            return -1;
        } else if (a.data[i] > b.data[i]) {
            return 1;
        }
    }
    if (a.length < b.length) {
        return -1;
    } else if (a.length > b.length) {
        return 1;
    } else {
        return 0;
    }
}

String resizeStringData(String string) {
    string.data = REALLOC(char, string.data, string.length + 1);
    string.data[string.length] = 0;
    return string;
}

String copyFromCString(const char* cstr) {
    return copyString(createFromConstCString(cstr));
}

static bool isHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return (int)(c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (int)(c - 'A') + 10;
    }
    return -1;
}

static CodePoint parseEscapeCode(char* data, size_t* length) {
    CodePoint ret;
    switch (data[0]) {
        case 'a':
            ret = '\a';
            *length = 1;
            break;
        case 'b':
            ret = '\b';
            *length = 1;
            break;
        case 't':
            ret = '\t';
            *length = 1;
            break;
        case 'n':
            ret = '\n';
            *length = 1;
            break;
        case 'v':
            ret = '\v';
            *length = 1;
            break;
        case 'f':
            ret = '\f';
            *length = 1;
            break;
        case 'r':
            ret = '\r';
            *length = 1;
            break;
        case 'e':
            ret = '\e';
            *length = 1;
            break;
        case 'x':
            if (isHexChar(data[1]) && isHexChar(data[2])) {
                ret = (hexCharToInt(data[1]) << 4) | hexCharToInt(data[2]);
                *length = 3;
            } else {
                ret = -1;
            }
            break;
        case 'u':
            if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4])) {
                ret = (hexCharToInt(data[1]) << 12) | (hexCharToInt(data[2]) << 8)
                      | (hexCharToInt(data[3]) << 4) | hexCharToInt(data[4]);
                *length = 5;
            } else {
                ret = -1;
            }
            break;
        case 'U':
            if (isHexChar(data[1]) && isHexChar(data[2]) && isHexChar(data[3]) && isHexChar(data[4]) && isHexChar(data[5]) && isHexChar(data[6]) && isHexChar(data[7]) && isHexChar(data[8])) {
                ret = (hexCharToInt(data[1]) << 28) | (hexCharToInt(data[2]) << 24)
                      | (hexCharToInt(data[3]) << 20) | (hexCharToInt(data[4]) << 16);
                ret |= (hexCharToInt(data[5]) << 12) | (hexCharToInt(data[6]) << 8)
                      | (hexCharToInt(data[7]) << 4) | hexCharToInt(data[8]);
                *length = 9;
            } else {
                ret = -1;
            }
            break;
        default:
            ret = data[0];
            *length = 1;
            break;
    }
    return ret;
}

void inlineDecodeStringLiteral(String* string) {
    size_t new = 0;
    size_t old = 1;
    while (string->data[old] != 0) {
        if (string->data[old] == '\\') {
            size_t length;
            CodePoint codepoint = parseEscapeCode(string->data + old + 1, &length);
            if (codepoint == INVALID_CODEPOINT) {
                // TODO: Throw error
                // addError(error_context, "Found an illegal escape code",
                // getCurrentScannerPosition(scanner), ERROR);
            } else {
                new += encodeUTF8(codepoint, string->data + new, length);
                old += length;
            }
            old++;
        } else {
            string->data[new] = string->data[old];
            new ++;
            old++;
        }
    }
    string->data[new - 1] = 0;
}

