
#include <stdbool.h>
#include <string.h>

#include "util/alloc.h"

#include "const/bigint.h"

#define WORD_SIZE 32
#define REALLOC_LIMIT 2

#define SIZE_FOR(SIZE) (sizeof(BigInt) + (SIZE) * sizeof(uint32_t))
#define TEMP_SMALLINT(NAME, VALUE)                          \
    char tmp_ ## NAME[sizeof(BigInt) + sizeof(uint32_t)];   \
    BigInt* NAME = (BigInt*)(tmp_ ## NAME);                 \
    NAME->size = 1;                                         \
    NAME->words[0] = VALUE;

static void absWordMulBigInt(BigInt** dst, uint32_t b) {
    if (b == 0) {
        (*dst)->size = 0;
    } else if ((*dst)->size != 0) {
        uint32_t carry = 0;
        for (size_t i = 0; i < (*dst)->size; i++) {
            uint64_t tmp = (uint64_t)b * (uint64_t)(*dst)->words[i] + (uint64_t)carry;
            (*dst)->words[i] = tmp;
            carry = tmp >> WORD_SIZE;
        }
        if (carry != 0) {
            *dst = checkedRealloc(*dst, SIZE_FOR((*dst)->size + 1));
            (*dst)->words[(*dst)->size] = carry; 
            (*dst)->size += 1;
        }
    }
}

static void absAddBigInt(BigInt** dst, BigInt* b, uint32_t shift) {
    if ((*dst)->size < b->size + shift) {
        *dst = checkedRealloc(*dst, SIZE_FOR(b->size + shift));
        for (size_t i = (*dst)->size; i < b->size + shift; i++) {
            (*dst)->words[i] = 0;
        }
        (*dst)->size = b->size + shift;
    }
    uint32_t carry = 0;
    size_t i = 0;
    for (; i < b->size; i++) {
        uint64_t tmp = (uint64_t)(*dst)->words[i + shift] + (uint64_t)b->words[i] + carry;
        (*dst)->words[i + shift] = tmp;
        carry = tmp >> WORD_SIZE;
    }
    for (; i + shift < (*dst)->size && carry != 0; i++) {
        uint64_t tmp = (uint64_t)(*dst)->words[i + shift] + carry;
        (*dst)->words[i + shift] = tmp;
        carry = tmp >> WORD_SIZE;
    }
    if (carry != 0) {
        *dst = checkedRealloc(*dst, SIZE_FOR((*dst)->size + 1));
        (*dst)->words[(*dst)->size] = carry; 
        (*dst)->size += 1;
    }
}

static void absWordAddBigInt(BigInt** dst, uint32_t b) {
    TEMP_SMALLINT(a, b);
    absAddBigInt(dst, a, 0);
}

BigInt* createBigInt() {
    BigInt* res = NEW(BigInt);
    res->size = 0;
    return res;
}

BigInt* createBigIntFrom(intmax_t value) {
    int32_t size = (sizeof(value) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    BigInt* res = checkedAlloc(SIZE_FOR(size));
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
    res->size = size;
    res->negative = value < 0;
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
    BigInt* res = createBigInt();
    size_t idx = 0;
    if (str.data[idx] == '-') {
        res->negative = true;
        idx++;
    } else {
        res->negative = false;
    }
    for (; idx < str.length; idx++) {
        int digit = digitCharToInt(str.data[idx]);
        if (digit != -1) {
            absWordMulBigInt(&res, base);
            absWordAddBigInt(&res, digit);
        }
    }
    res->size = res->size;
    return res;
}

BigInt* copyBigInt(BigInt* bi) {
    size_t size = SIZE_FOR(bi->size);
    BigInt* res = checkedAlloc(size);
    memcpy(res, bi, size);
    return res;
}

void freeBigInt(BigInt* bi) {
    FREE(bi);
}

int signOfBigInt(BigInt* bi) {
    if (bi->size > 0) {
        return bi->negative ? -1 : 1;
    } else {
        return 0;
    }
}

static int absCompareBigInt(BigInt* a, BigInt* b) {
    if (a->size != b->size) {
        return a->size - b->size;
    } else {
        for (size_t i = a->size; i > 0;) {
            i--;
            if (a->words[i] != b->words[i]) {
                return a->words[i] - b->words[i];
            }
        }
        return 0;
    }
}

int compareBigInt(BigInt* a, BigInt* b) {
    if (signOfBigInt(a) != signOfBigInt(b)) {
        return signOfBigInt(a) - signOfBigInt(b);
    } else if (a->negative) {
        return -absCompareBigInt(a, b);
    } else {
        return absCompareBigInt(a, b);
    }
}

BigInt* negBigInt(BigInt* bi) {
    BigInt* res = copyBigInt(bi);
    res->negative = !res->negative;
    return res;
}

BigInt* addBigInt(BigInt* a, BigInt* b) {
    // TODO
}

BigInt* subBigInt(BigInt* a, BigInt* b) {
    // TODO
}

static void absMulBigInt(BigInt** dst, BigInt* b) {
    if (b->size == 0 || (*dst)->size == 0) {
        if ((*dst)->size > REALLOC_LIMIT) {
            *dst = checkedRealloc(*dst, SIZE_FOR(0));
        }
        (*dst)->size = 0;
    } else if (b->size == 1) {
        absWordMulBigInt(dst, b->words[0]);
    } else {
        BigInt* copy = copyBigInt(*dst);
        absWordMulBigInt(dst, b->words[0]);
        for (size_t i = 1; i < b->size; i++) {
            BigInt* tmp = i == b->size - 1 ? copy : copyBigInt(copy);
            absWordMulBigInt(&tmp, b->words[i]);
            absAddBigInt(dst, tmp, i);
            freeBigInt(tmp);
        }
    }
}

BigInt* mulBigInt(BigInt* a, BigInt* b) {
    BigInt* result = copyBigInt(a);
    result->negative = a->negative != b->negative;
    absMulBigInt(&result, b);
    return result;
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

