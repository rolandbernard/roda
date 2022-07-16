
#include <stdbool.h>
#include <string.h>

#include "errors/fatalerror.h"
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
        *dst = checkedRealloc(*dst, SIZE_FOR(b->size + shift + 1));
        for (size_t i = (*dst)->size; i < b->size + shift + 1; i++) {
            (*dst)->words[i] = 0;
        }
        (*dst)->size = b->size + shift + 1;
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
    } else {
        while ((*dst)->size > 0 && (*dst)->words[(*dst)->size - 1] == 0) {
            (*dst)->size--;
        }
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

bool isZero(BigInt* a) {
    return a->size == 0;
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

BigInt* absBigInt(BigInt* bi) {
    BigInt* res = copyBigInt(bi);
    res->negative = false;
    return res;
}

static void absSubBigInt(BigInt** dst, BigInt* b) {
    // The value in `*dst` must be greather than that in `b`
    uint32_t carry = 0;
    size_t i = 0;
    for (; i < b->size; i++) {
        uint64_t to_sub = (uint64_t)b->words[i] + carry;
        if (to_sub > (*dst)->words[i]) {
            (*dst)->words[i] = ((uint64_t)1 << WORD_SIZE) + (*dst)->words[i] - to_sub;
            carry = 1;
        } else {
            (*dst)->words[i] = (*dst)->words[i] - to_sub;
            carry = 0;
        }
    }
    for (; i < (*dst)->size && carry != 0; i++) {
        uint64_t tmp = (uint64_t)(*dst)->words[i] + carry;
        (*dst)->words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
    while ((*dst)->size > 0 && (*dst)->words[(*dst)->size - 1] == 0) {
        (*dst)->size--;
    }
}

static BigInt* absDifference(BigInt* a, BigInt* b) {
    int cmp = absCompareBigInt(a, b);
    if (cmp == -1) {
        BigInt* res = copyBigInt(b);
        absSubBigInt(&res, a);
        return res;
    } else if (cmp == 1) {
        BigInt* res = copyBigInt(a);
        absSubBigInt(&res, b);
        return res;
    } else {
        return createBigInt();
    }
}

BigInt* addBigInt(BigInt* a, BigInt* b) {
    if (a->negative == b->negative) {
        BigInt* res = copyBigInt(a);
        absAddBigInt(&res, b, 0);
        return res;
    } else {
        return absDifference(a, b);
    }
}

BigInt* subBigInt(BigInt* a, BigInt* b) {
    if (a->negative != b->negative) {
        BigInt* res = copyBigInt(a);
        absAddBigInt(&res, b, 0);
        return res;
    } else {
        return absDifference(a, b);
    }
}

static BigInt* copyBigIntSubrange(BigInt* bi, uint32_t offset, uint32_t length) {
    size_t size = SIZE_FOR(length);
    BigInt* res = checkedAlloc(size);
    res->negative = bi->negative;
    res->size = length;
    memcpy(res->words, bi->words + offset, length * sizeof(uint32_t));
    return res;
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
    BigInt* res = copyBigInt(a);
    res->negative = a->negative != b->negative;
    absMulBigInt(&res, b);
    return res;
}

static void absWordDivBigInt(BigInt** dst, uint32_t b) {
    uint32_t carry = 0;
    for (size_t i = (*dst)->size; i > 0;) {
        i--;
        uint64_t tmp = (uint64_t)(*dst)->words[i] + ((uint64_t)carry << WORD_SIZE);
        (*dst)->words[i] = tmp / b;
        carry = tmp % b;
    }
    while ((*dst)->size > 0 && (*dst)->words[(*dst)->size - 1] == 0) {
        (*dst)->size--;
    }
}

BigInt* divBigInt(BigInt* a, BigInt* b) {
    ASSERT(b->size != 0);
    if (a->size == 0 || a->size < b->size) {
        return createBigInt();
    } else if (b->size == 1) {
        BigInt* res = copyBigInt(a);
        absWordDivBigInt(&res, b->words[0]);
        res->negative = a->negative != b->negative;
        return res;
    } else {
        // TODO
    }
}

static uint32_t absWordRemBigInt(BigInt* a, uint32_t b) {
    uint32_t carry = 0;
    for (size_t i = a->size; i > 0;) {
        i--;
        uint64_t tmp = (uint64_t)a->words[i] + ((uint64_t)carry << WORD_SIZE);
        carry = tmp % b;
    }
    return carry;
}

BigInt* remBigInt(BigInt* a, BigInt* b) {
    ASSERT(b->size != 0);
    if (a->size == 0) {
        return createBigInt();
    } else if (a->size < b->size) {
        return copyBigInt(a);
    } else if (b->size == 1) {
        BigInt* res = createBigIntFrom(absWordRemBigInt(a, b->words[0]));
        res->negative = a->negative != b->negative;
        return res;
    } else {
        // TODO
    }
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

static char digitIntToChar(int i) {
    if (i >= 0 && i <= 9) {
        return '0' + i;
    } else if (i >= 0 && i <= 36) {
        return 'a' + i - 10;
    } else {
        return '?';
    }
}

String stringForBigInt(BigInt* bi, uint32_t base) {
    if (isZero(bi)) {
        return copyFromCString("0");
    } else {
        StringBuilder builder;
        initStringBuilder(&builder);
        BigInt* copy = absBigInt(bi);
        while (!isZero(copy)) {
            int digit = absWordRemBigInt(copy, base);
            absWordDivBigInt(&copy, base);
            pushCharToStringBuilder(&builder, digitIntToChar(digit));
        }
        freeBigInt(copy);
        if (signOfBigInt(bi) < 0) {
            pushCharToStringBuilder(&builder, '-');
        }
        reverseStringBuilder(&builder);
        return builderToString(&builder);
    }
}

