
#include <stdbool.h>
#include <string.h>

#include "util/alloc.h"

#include "const/bigint.h"

#define WORD_SIZE 32

#define TEMP_SMALLINT(NAME, VALUE)                          \
    char tmp_ ## NAME[sizeof(BigInt) + sizeof(uint32_t)];   \
    BigInt* NAME = (BigInt*)(tmp_ ## NAME);                 \
    NAME->size = 1;                                         \
    NAME->words[0] = VALUE;

static int32_t sizeOf(BigInt* bi) {
    return bi->size < 0 ? -bi->size : bi->size;
}

static void absMulBigInt(BigInt** dst, BigInt* b) {
    // TODO
}

static void absWordMulBigInt(BigInt** dst, uint32_t b) {
    TEMP_SMALLINT(a, b);
    absMulBigInt(dst, a);
}

static void absAddBigInt(BigInt** dst, BigInt* b) {
    // TODO
}

static void absWordAddBigInt(BigInt** dst, uint32_t b) {
    TEMP_SMALLINT(a, b);
    absAddBigInt(dst, a);
}

BigInt* createBigInt() {
    BigInt* res = NEW(BigInt);
    res->size = 0;
    return res;
}

BigInt* createBigIntFrom(intmax_t value) {
    int32_t size = (sizeof(value) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    BigInt* res = checkedAlloc(sizeof(BigInt) + size * sizeof(uint32_t));
    uintmax_t pos;
    if (value < 0) {
        pos = -value;
    } else {
        pos = value;
    }
    size = 0;
    while (pos > 0) {
        res->words[size] = pos;
        pos >>= WORD_SIZE;
    }
    if (value < 0) {
        res->size = -size;
    } else {
        res->size = size;
    }
    return res;
}

static int digitCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    } else if (c >= 'a' && c <= 'z') {
        return (int)(c - 'a') + 10;
    } else if (c >= 'A' && c <= 'Z') {
        return (int)(c - 'A') + 10;
    }
    return -1;
}

BigInt* createBigIntFromString(ConstString str, uint32_t base) {
    size_t idx = 0;
    bool neg = false;
    if (str.data[idx] == '-') {
        neg = true;
        idx++;
    }
    BigInt* res = createBigInt();
    for (; idx < str.length; idx++) {
        int digit = digitCharToInt(str.data[idx]);
        if (digit != -1) {
            absWordMulBigInt(&res, base);
            absWordAddBigInt(&res, digit);
        }
    }
    if (neg) {
        res->size = -res->size;
    }
    return res;
}

BigInt* copyBigInt(BigInt* bi) {
    size_t size = sizeof(BigInt) + sizeOf(bi) * sizeof(uint32_t);
    BigInt* res = checkedAlloc(size);
    memcpy(res, bi, size);
    return res;
}

void freeBigInt(BigInt* bi) {
    FREE(bi);
}

int signOfBigInt(BigInt* bi) {
    if (bi->size > 0) {
        return 1;
    } else if (bi->size < 0) {
        return -1;
    } else {
        return 0;
    }
}

int compareBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* negBigInt(BigInt* bi) {
    BigInt* res = copyBigInt(bi);
    res->size = -res->size;
    return res;
}

BigInt* addBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* subBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* mulBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* divBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* remBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* andBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* orBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* xorBigInt(BigInt* a, BigInt* b) {
    // TODO
}

String stringForBigInt(BigInt* bi, uint32_t base) {
    // TODO
}

