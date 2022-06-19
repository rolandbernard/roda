
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "text/format.h"

#include "util/alloc.h"

String createFormattedString(const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char* data = ALLOC(char, size + 1);
    va_start(args, format);
    vsnprintf(data, size + 1, format, args);
    va_end(args);
    data[size] = 0;
    return createString(data, size);
}

void pushFormattedString(String* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t size = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char* data = ALLOC(char, size + 1);
    va_start(args, format);
    vsnprintf(data, size + 1, format, args);
    va_end(args);
    data[size] = 0;
    *dst = pushToString(*dst, createConstString(data, size));
    FREE(data);
}

