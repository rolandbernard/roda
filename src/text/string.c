
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "text/string.h"
#include "errors/fatalerror.h"

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
    return copyFromCString("");
}

ConstString createEmptyConstString() {
    return createFromConstCString("");
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

void freeString(String string) {
    FREE(string.data);
}

const char* toCString(ConstString string) {
    ASSERT(string.data[string.length] == 0); // If this fails, you must create a copy!
    return string.data;
}

char* copyToCString(ConstString string) {
    char* ret = ALLOC(char, string.length + 1);
    memcpy(ret, string.data, string.length * sizeof(char));
    ret[string.length] = 0;
    return ret;
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

#define INITIAL_CAPACITY 32

void initStringBuilder(StringBuilder* builder) {
    builder->data = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

void deinitStringBuilder(StringBuilder* builder) {
    FREE(builder->data);
}

void makeSpaceInStringBuilder(StringBuilder* builder, size_t length) {
    if (builder->length + length + 1 > builder->capacity) {
        while (builder->capacity < builder->length + length + 1) {
            builder->capacity = builder->capacity == 0 ? INITIAL_CAPACITY : 2 * builder->capacity;
        }
        builder->data = REALLOC(char, builder->data, builder->capacity);
    }
}

void pushToStringBuilder(StringBuilder* builder, ConstString src) {
    makeSpaceInStringBuilder(builder, src.length);
    memcpy(builder->data + builder->length, src.data, src.length);
    builder->length += src.length;
    builder->data[builder->length] = 0;
}

String builderToString(StringBuilder* builder) {
    return createString(builder->data, builder->length);
}

