
#include <string.h>

#include "util/text.h"

static uint32_t parseEscapeCode(char* data, int* length) {
    uint32_t ret;
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

bool isHexChar(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

uint32_t hexCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return (int)(c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (int)(c - 'A') + 10;
    }
    return -1;
}

void inlineDecodeStringLiteral(char* string) {
    size_t new = 0;
    size_t old = 1;
    while (string[old] != 0) {
        if (string[old] == '\\') {
            int length;
            int codepoint = parseEscapeCode(string + old + 1, &length);
            if (codepoint == -1) {
                // TODO: Throw error
                // addError(error_context, "Found an illegal escape code",
                // getCurrentScannerPosition(scanner), ERROR);
            } else {
                new += printUTF8(codepoint, string + new);
                old += length;
            }
            old++;
        } else {
            string[new] = string[old];
            new ++;
            old++;
        }
    }
    string[new - 1] = 0;
}

uint32_t parseUTF8(char* c, size_t* length) {
    if ((c[0] & 0x80) == 0) {
        *length = 1;
        return c[0];
    } else {
        uint32_t ret = 0;
        size_t i = 0;
        while (i < 8 && (c[0] & (1 << (7 - i))) != 0) {
            i++;
        }
        *length = i;
        ret |= (c[0] & (0xff >> (6 - i))) << ((i - 1) * 6);
        size_t j = 0;
        while (i > 1) {
            i--;
            j++;
            if ((c[j] & 0xc0) != 0x80) {
                return -1;
            }
            ret |= (c[j] & 0x3f) << ((i - 1) * 6);
        }
        return ret;
    }
}

size_t printUTF8(uint32_t c, char* out) {
    if (c <= 0x7f) {
        out[0] = (char)c;
        return 1;
    } else {
        size_t i = 2;
        while (c > (1 << ((i - 1) * 6 + (6 - i)))) {
            i++;
        }
        out[0] = 0;
        for (size_t j = 0; j < i; j++) {
            out[0] |= (1 << (7 - j));
        }
        out[0] |= (c >> ((i - 1) * 6));
        for (size_t j = 1; j < i; j++) {
            out[j] = 0x80 | ((c >> ((i - j - 1) * 6)) & 0x3f);
        }
        return i;
    }
}
