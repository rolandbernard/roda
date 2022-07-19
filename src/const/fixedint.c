
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

static void inlineMulWordFixedInt(FixedInt* dst, uint32_t word) {
    uint32_t carry = 0;
    for (size_t i = 0; i < WORDS(dst->size); i++) {
        uint64_t tmp = (uint64_t)dst->words[i] * (uint64_t)word + carry;
        dst->words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
}

static void inlineAddWordFixedInt(FixedInt* dst, uint32_t word) {
    uint32_t carry = word;
    for (size_t i = 0; i < WORDS(dst->size) && carry != 0; i++) {
        uint64_t tmp = (uint64_t)dst->words[i] + (uint64_t)carry;
        dst->words[i] = tmp;
        carry = tmp >> WORD_SIZE;
    }
}

static void inlineNotFixedInt(FixedInt* dst) {
    for (size_t i = 0; i < WORDS(dst->size); i++) {
        dst->words[i] = ~dst->words[i];
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
            inlineMulWordFixedInt(res, base);
            inlineAddWordFixedInt(res, digit);
        }
    }
    if (str.data[0] == '-') {
        inlineNotFixedInt(res);
        inlineAddWordFixedInt(res, 1);
    }
    return res;
}

FixedInt* createFixedIntFromBigInt(uint32_t size, BigInt* bi) {
    FixedInt* res = createFixedInt(size);
    for (size_t i = 0; i * WORD_SIZE < size && i < bi->size; i++) {
        res->words[i] = bi->words[i];
    }
    if (bi->negative) {
        inlineNotFixedInt(res);
        inlineAddWordFixedInt(res, 1);
    }
    return res;
}

BigInt* createBigIntFromFixedIntZeroExtend(FixedInt* fi) {
    size_t length = WORDS(fi->size);
    while (length > 0 && fi->words[length - 1] == 0) {
        length--;
    }
    BigInt* res = createBigIntCapacity(length);
    res->size = length;
    memcpy(res->words, fi->words, length * sizeof(uint32_t));
    return res;
}

/* BigInt* createBigIntFromFixedIntSignExtend(FixedInt* fi); */

/* FixedInt* resizeFixedIntZeroExtend(FixedInt* fi, uint32_t size); */

/* FixedInt* resizeFixedIntSignExtend(FixedInt* fi, uint32_t size); */

/* FixedInt* copyFixedInt(FixedInt* fi); */

/* void freeFixedInt(FixedInt* fi); */

/* int signOfFixedInt(FixedInt* fi); */

/* bool isFixedIntZero(FixedInt* a); */

/* int compareFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* negFixedInt(FixedInt* fi); */

/* FixedInt* absFixedInt(FixedInt* fi); */

/* FixedInt* addFixedInt(FixedInt* a, FixedInt* b); */

/* FixedInt* subFixedInt(FixedInt* a, FixedInt* b); */

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

