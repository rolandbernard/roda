
#include <string.h>

#include "util/text.h"

char* decodeEscapeCharacters(char* dst, const char* string) {
    while (*string != 0) {
        if (*string == '\\' && string[1] != 0) {
            switch (string[1]) {
                case 'n':
                    *dst = '\n';
                    break;
                case 't':
                    *dst = '\t';
                    break;
                case 'r':
                    *dst = '\r';
                    break;
                case 'f':
                    *dst = '\f';
                    break;
                case 'v':
                    *dst = '\v';
                    break;
                case 'a':
                    *dst = '\a';
                    break;
                case 'b':
                    *dst = '\b';
                    break;
                default:
                    *dst = string[1];
                    break;
            }
            string++;
        } else {
            *dst = *string;
        }
        string++;
        dst++;
    }
    *dst = 0;
    return dst;
}

void inlineDecodeStringLiteral(char* string) {
    string[strlen(string) - 1] = 0;
    decodeEscapeCharacters(string, string + 1);
}

