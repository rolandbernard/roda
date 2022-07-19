
#include "util/util.h"

bool isDigitChar(char c, int base) {
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
           && digitCharToInt(c) < base;
}

int digitCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    } else if (c >= 'a' && c <= 'z') {
        return (int)(c - 'a') + 10;
    } else if (c >= 'A' && c <= 'Z') {
        return (int)(c - 'A') + 10;
    }
    return -1;
}

char digitIntToChar(int i) {
    if (i >= 0 && i <= 9) {
        return '0' + i;
    } else if (i >= 0 && i <= 36) {
        return 'a' + i - 10;
    } else {
        return '?';
    }
}

