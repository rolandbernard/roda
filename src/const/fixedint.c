
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/util.h"

#include "const/fixedint.h"

#define WORD_SIZE 32

#define TEMP_SMALLINT(NAME, VALUE)                          \
    char tmp_ ## NAME[sizeof(FixedInt) + sizeof(uint32_t)]; \
    FixedInt* NAME = (FixedInt*)(tmp_ ## NAME);             \
    NAME->size = 1;                                         \
    NAME->words[0] = VALUE;

#define WORDS(BITS) (((BITS) + WORD_SIZE - 1) / WORD_SIZE)

FixedInt* createFixedInt(uint32_t size) {
    FixedInt* res = checkedCalloc(sizeof(FixedInt) + WORDS(size) * sizeof(uint32_t), 1);
    res->size = size;
    return res;
}

FixedInt* createFixedIntFrom(uint32_t size, intmax_t value) {
    FixedInt* res = createFixedInt(size);
    for (size_t i = 0; i < WORDS(size) && value != 0; i++) {
        res->words[i] = value;
        value >>= WORD_SIZE;
    }
    return res;
}

FixedInt* createFixedIntFromUnsigned(uint32_t size, uintmax_t value) {
    FixedInt* res = createFixedInt(size);
    for (size_t i = 0; i < WORDS(size) && value != 0; i++) {
        res->words[i] = value;
        value >>= WORD_SIZE;
    }
    return res;
}

static void inlineMulWordFixedInt(uint32_t* words, uint32_t size, uint32_t word) {
    uint32_t carry = 0;
    for (size_t i = 0; i < WORDS(size); i++) {
        uint64_t tmp = (uint64_t)words[i] * (uint64_t)word + carry;
        words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
}

static void inlineAddWordFixedInt(uint32_t* words, uint32_t size, uint32_t word) {
    uint32_t carry = word;
    for (size_t i = 0; i < WORDS(size) && carry != 0; i++) {
        uint64_t tmp = (uint64_t)words[i] + (uint64_t)carry;
        words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
}

static void inlineNotFixedInt(uint32_t* words, uint32_t size) {
    for (size_t i = 0; i < WORDS(size); i++) {
        words[i] = ~words[i];
    }
}

FixedInt* createFixedIntFromString(uint32_t size, ConstString str, int base) {
    FixedInt* res = createFixedInt(size);
    size_t idx = 0;
    if (str.data[idx] == '-') {
        idx++;
    }
    for (; idx < str.length; idx++) {
        int digit = digitCharToInt(str.data[idx]);
        if (digit != -1) {
            inlineMulWordFixedInt(res->words, res->size, base);
            inlineAddWordFixedInt(res->words, res->size, digit);
        }
    }
    if (str.data[0] == '-') {
        inlineNotFixedInt(res->words, res->size);
        inlineAddWordFixedInt(res->words, res->size, 1);
    }
    return res;
}

FixedInt* createFixedIntFromBigInt(uint32_t size, BigInt* bi) {
    FixedInt* res = createFixedInt(size);
    for (size_t i = 0; i < WORDS(size) && i < bi->size; i++) {
        res->words[i] = bi->words[i];
    }
    if (bi->negative) {
        inlineNotFixedInt(res->words, res->size);
        inlineAddWordFixedInt(res->words, res->size, 1);
    }
    return res;
}

static int signBitOfFixedInt(uint32_t* words, uint32_t size) {
    return (words[(size - 1) / WORD_SIZE] >> ((size - 1) % WORD_SIZE)) & 1;
}

static void inlineZeroExtendToFullWord(uint32_t* words, uint32_t size) {
    if (size % WORD_SIZE != 0) {
        words[size / WORD_SIZE] &= ~((~(uint32_t)0) << (size % WORD_SIZE));
    }
}

static void inlineSignExtendToFullWord(uint32_t* words, uint32_t size) {
    if (size % WORD_SIZE != 0) {
        if (signBitOfFixedInt(words, size) == 0) {
            words[size / WORD_SIZE] &= ~((~(uint32_t)0) << (size % WORD_SIZE));
        } else {
            words[size / WORD_SIZE] |= ((~(uint32_t)0) << (size % WORD_SIZE));
        }
    }
}

BigInt* createBigIntFromFixedIntZeroExtend(FixedInt* fi) {
    size_t length = WORDS(fi->size);
    BigInt* res = createBigIntCapacity(length);
    for (size_t i = 0; i < length; i++) {
        res->words[i] = fi->words[i];
    }
    inlineZeroExtendToFullWord(res->words, fi->size);
    while (length > 0 && res->words[length - 1] == 0) {
        length--;
    }
    res->size = length;
    return res;
}

BigInt* createBigIntFromFixedIntSignExtend(FixedInt* fi) {
    if (signBitOfFixedInt(fi->words, fi->size) == 0) {
        return createBigIntFromFixedIntZeroExtend(fi);
    } else {
        size_t length = WORDS(fi->size);
        BigInt* res = createBigIntCapacity(length + 1);
        for (size_t i = 0; i < length; i++) {
            res->words[i] = fi->words[i];
        }
        inlineSignExtendToFullWord(res->words, fi->size);
        while (length > 0 && ~fi->words[length - 1] == 0) {
            length--;
        }
        uint32_t carry = 1;
        for (size_t i = 0; i < length; i++) {
            res->words[i] = ~res->words[i] + carry;
            carry = res->words[i] == 0 ? 1 : 0;
        }
        if (carry != 0) {
            res->words[length] = carry;
            res->size = length + 1;
        } else {
            res->size = length;
        }
        return res;
    }
}

FixedInt* resizeFixedIntZeroExtend(FixedInt* fi, uint32_t size) {
    FixedInt* res = createFixedInt(size);
    size_t old_len = WORDS(fi->size);
    size_t new_len = WORDS(size);
    for (size_t i = 0; i < old_len && i < new_len; i++) {
        res->words[i] = fi->words[i];
    }
    if (size > fi->size) {
        inlineZeroExtendToFullWord(res->words, fi->size);
    }
    return res;
}

FixedInt* resizeFixedIntSignExtend(FixedInt* fi, uint32_t size) {
    FixedInt* res = createFixedInt(size);
    size_t old_len = WORDS(fi->size);
    size_t new_len = WORDS(size);
    for (size_t i = 0; i < old_len && i < new_len; i++) {
        res->words[i] = fi->words[i];
    }
    if (size > fi->size) {
        inlineSignExtendToFullWord(res->words, fi->size);
        uint32_t extend = (signBitOfFixedInt(fi->words, fi->size) == 0 ? 0 : ~(uint32_t)0);
        for (size_t i = old_len; i < new_len; i++) {
            res->words[i] = extend;
        }
    }
    return res;
}

FixedInt* copyFixedInt(FixedInt* fi) {
    FixedInt* res = createFixedInt(fi->size);
    memcpy(res, fi, sizeof(FixedInt) + WORDS(fi->size) * sizeof(uint32_t));
    return res;
}

void freeFixedInt(FixedInt* fi) {
    FREE(fi);
}

bool isFixedIntZero(FixedInt* a) {
    for (size_t i = 0; i < WORDS(a->size); i++) {
        uint32_t w = a->words[i];
        if ((i + 1) * WORD_SIZE > a->size) {
            if (signBitOfFixedInt(a->words, a->size) == 0) {
                w &= ~((~(uint32_t)0) << (a->size % WORD_SIZE));
            } else {
                w |= ((~(uint32_t)0) << (a->size % WORD_SIZE));
            }
        }
        if (w != 0) {
            return false;
        }
    }
    return true;
}

int signOfFixedInt(FixedInt* fi) {
    if (isFixedIntZero(fi)) {
        return 0;
    } else if (signBitOfFixedInt(fi->words, fi->size) == 1) {
        return -1;
    } else {
        return 1;
    }
}

static int absCompareFixedInt(FixedInt* a, FixedInt* b) {
    for (size_t i = WORDS(a->size); i > 0;) {
        i--;
        uint32_t wa = a->words[i];
        uint32_t wb = b->words[i];
        if ((i + 1) * WORD_SIZE > a->size) {
            wa &= ~((~(uint32_t)0) << (a->size % WORD_SIZE));
            wb &= ~((~(uint32_t)0) << (a->size % WORD_SIZE));
        }
        if (wa != wb) {
            return wa > wb ? 1 : -1;
        }
    }
    return 0;
}

int compareFixedIntSigned(FixedInt* a, FixedInt* b) {
    ASSERT(a->size == b->size);
    int sign_a = signBitOfFixedInt(a->words, a->size);
    int sign_b = signBitOfFixedInt(b->words, b->size);
    if (sign_a != sign_b) {
        return sign_a == 0 ? 1 : -1;
    } else {
        return absCompareFixedInt(a, b);
    }
}

int compareFixedIntUnsigned(FixedInt* a, FixedInt* b) {
    ASSERT(a->size == b->size);
    return absCompareFixedInt(a, b);
}

FixedInt* negFixedInt(FixedInt* fi) {
    FixedInt* res = copyFixedInt(fi);
    inlineNotFixedInt(res->words, res->size);
    inlineAddWordFixedInt(res->words, res->size, 1);
    return res;
}

FixedInt* absFixedInt(FixedInt* fi) {
    if (signBitOfFixedInt(fi->words, fi->size) == 1) {
        return negFixedInt(fi);
    } else {
        return copyFixedInt(fi);
    }
}

FixedInt* addFixedInt(FixedInt* a, FixedInt* b) {
    ASSERT(a->size == b->size);
    FixedInt* res = copyFixedInt(a);
    uint32_t carry = 0;
    for (size_t i = 0; i < WORDS(a->size); i++) {
        uint64_t tmp = (uint64_t)res->words[i] + (uint64_t)b->words[i] + (uint64_t)carry;
        res->words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
    return res;
}

FixedInt* subFixedInt(FixedInt* a, FixedInt* b) {
    ASSERT(a->size == b->size);
    FixedInt* res = copyFixedInt(a);
    uint32_t carry = 1;
    for (size_t i = 0; i < WORDS(a->size); i++) {
        uint64_t tmp = (uint64_t)res->words[i] + (uint64_t)~b->words[i] + (uint64_t)carry;
        res->words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
    return res;
}

/* FixedInt* mulFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* udivFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* sdivFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* uremFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* sremFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* andFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* orFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* xorFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* notFixedInt(FixedInt* a); */

/* FixedInt* shiftLeftLogicalFixedInt(FixedInt* a, size_t r); */

/* FixedInt* shiftLeftArithmeticFixedInt(FixedInt* a, size_t r); */

/* FixedInt* shiftRightFixedInt(FixedInt* a, size_t r); */

/* String stringForFixedInt(FixedInt* fi, int base); */

/* intmax_t intMaxForFixedInt(FixedInt* fi); */

/* uintmax_t uintMaxForFixedInt(FixedInt* fi); */

