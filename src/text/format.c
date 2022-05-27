
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "text/format.h"

#include "util/alloc.h"

static int formatCustomString(char* output, const char* format, va_list args) {
    int size = 0;
    for (int i = 0; format[i] != 0; i++) {
        if (format[i] == '%') {
            if (format[i + 1] == 'S') {
                i++;
                ConstString value = va_arg(args, ConstString);
                if (output != NULL) {
                    memcpy(output + size, value.data, value.length);
                }
                size += value.length;
            } else {
                int end = i + 1;
                while (format[end] != 0 && format[end] != '%' && !isalpha(format[end])) {
                    end++;
                }
                if (format[end] == '%' || isalpha(format[end])) {
                    char tmp[end - i + 2];
                    memcpy(tmp, format + i, end - i + 1);
                    tmp[end - i + 1] = 0;
                    if (output != NULL) {
                        size += vsprintf(output + size, tmp, args);
                    } else {
                        size += vsnprintf(NULL, 0, tmp, args);
                    }
                    i = end;
                }
            }
        } else {
            if (output != NULL) {
                output[size] = format[i];
            }
            size++;
        }
    }
    return size;
}

String createFormatedString(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int size = formatCustomString(NULL, format, args);
    va_end(args);
    char* data = ALLOC(char, size + 1);
    va_start(args, format);
    formatCustomString(data, format, args);
    va_end(args);
    return createString(data, size);
}
