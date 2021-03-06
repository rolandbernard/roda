
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/util.h"

#include "const/bigint.h"

#define WORD_SIZE 32
#define REALLOC_LIMIT 2

#define SIZE_FOR(SIZE) (sizeof(BigInt) + (SIZE) * sizeof(uint32_t))
#define TEMP_SMALLINT(NAME, VALUE)                          \
    char tmp_ ## NAME[sizeof(BigInt) + sizeof(uint32_t)];   \
    BigInt* NAME = (BigInt*)(tmp_ ## NAME);                 \
    NAME->size = 1;                                         \
    NAME->words[0] = VALUE;

static BigInt* reallocIfNeeded(BigInt* a, uint32_t cur) {
    if (cur - a->size > REALLOC_LIMIT) {
        return checkedRealloc(a, SIZE_FOR(a->size));
    } else {
        return a;
    }
}

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
    if (b->size != 0) {
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
}

static void absWordAddBigInt(BigInt** dst, uint32_t b) {
    TEMP_SMALLINT(a, b);
    absAddBigInt(dst, a, 0);
}

BigInt* createBigInt() {
    BigInt* res = NEW(BigInt);
    res->negative = false;
    res->size = 0;
    return res;
}

BigInt* createBigIntCapacity(uint32_t size) {
    BigInt* res = checkedAlloc(SIZE_FOR(size));
    res->negative = false;
    res->size = 0;
    return res;
}

BigInt* createBigIntFromUnsigned(uintmax_t value) {
    int32_t size = (sizeof(intmax_t) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    BigInt* res = checkedAlloc(SIZE_FOR(size));
    size = 0;
    while (value > 0) {
        res->words[size] = value;
        value >>= WORD_SIZE;
        size++;
    }
    res->size = size;
    res->negative = false;
    return res;
}

BigInt* createBigIntFrom(intmax_t value) {
    BigInt* res = createBigIntFromUnsigned(ABS(value));
    res->negative = value < 0;
    return res;
}

BigInt* createBigIntFromString(ConstString str, int base) {
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
        if (digit != -1 && digit < base) {
            absWordMulBigInt(&res, base);
            absWordAddBigInt(&res, digit);
        }
    }
    return res;
}

BigInt* createBigIntFromDouble(double value) {
    double abs = ABS(value);
    size_t len = 0;
    while (abs > 0) {
        len++;
        abs /= 1UL << WORD_SIZE;
    }
    abs = ABS(value);
    BigInt* res = createBigIntCapacity(len);
    for (size_t i = 0; i < len; i++) {
        res->words[i] = fmod(abs, 1UL << WORD_SIZE);
        abs /= 1UL << WORD_SIZE;
    }
    res->size = len;
    res->negative = value < 0;
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

bool isBigIntZero(BigInt* a) {
    return a->size == 0;
}

static int absCompareBigInt(BigInt* a, BigInt* b) {
    if (a->size != b->size) {
        return a->size > b->size ? 1 : -1;
    } else {
        for (size_t i = a->size; i > 0;) {
            i--;
            if (a->words[i] != b->words[i]) {
                return a->words[i] > b->words[i] ? 1 : -1;
            }
        }
        return 0;
    }
}

int compareBigInt(BigInt* a, BigInt* b) {
    if (signOfBigInt(a) != signOfBigInt(b)) {
        return signOfBigInt(a) > signOfBigInt(b) ? 1 : -1;
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
        if (carry > (*dst)->words[i]) {
            (*dst)->words[i] = ((uint64_t)1 << WORD_SIZE) - carry;
            carry = 1;
        } else {
            (*dst)->words[i] = (*dst)->words[i] - carry;
            carry = 0;
        }
    }
    while ((*dst)->size > 0 && (*dst)->words[(*dst)->size - 1] == 0) {
        (*dst)->size--;
    }
}

static void absWordSubBigInt(BigInt** dst, uint32_t b) {
    TEMP_SMALLINT(a, b);
    absSubBigInt(dst, a);
}

static BigInt* absDifference(BigInt* a, BigInt* b) {
    int cmp = absCompareBigInt(a, b);
    if (cmp < 0) {
        BigInt* res = copyBigInt(b);
        absSubBigInt(&res, a);
        return reallocIfNeeded(res, b->size);
    } else if (cmp > 0) {
        BigInt* res = copyBigInt(a);
        absSubBigInt(&res, b);
        return reallocIfNeeded(res, a->size);
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
        BigInt* res = absDifference(a, b);
        if (absCompareBigInt(a, b) < 0) {
            res->negative = !res->negative;
        }
        return res;
    }
}

static BigInt* copyBigIntSubrange(BigInt* bi, uint32_t offset, uint32_t length) {
    if (offset < bi->size) {
        length = MIN(length, bi->size - offset);
        while (length > 0 && bi->words[offset + length - 1] == 0) {
            length--;
        }
        BigInt* res = checkedAlloc(SIZE_FOR(length));
        res->negative = bi->negative;
        res->size = length;
        memcpy(res->words, bi->words + offset, length * sizeof(uint32_t));
        return res;
    } else {
        return createBigInt();
    }
}

static BigInt* absMulBigInt(BigInt* a, BigInt* b);

static BigInt* absMulBigIntBigSmall(BigInt* a, BigInt* b) {
    if (b->size < 32) {
        BigInt* res = copyBigInt(a);
        absWordMulBigInt(&res, b->words[0]);
        for (size_t i = 1; i < b->size; i++) {
            BigInt* tmp = copyBigInt(a);
            absWordMulBigInt(&tmp, b->words[i]);
            absAddBigInt(&res, tmp, i);
            freeBigInt(tmp);
        }
        return res;
    } else if (a->size / 2 > b->size) {
        BigInt* res = createBigInt();
        uint32_t segs = a->size / b->size;
        uint32_t offset = 0;
        for (size_t i = 0; i < segs; i++) {
            uint32_t length = i == segs - 1 ? a->size - offset : a->size / segs;
            BigInt* seg = copyBigIntSubrange(a, offset, length);
            BigInt* mul = absMulBigInt(seg, b);
            absAddBigInt(&res, mul, offset);
            freeBigInt(seg);
            freeBigInt(mul);
            offset += length;
        }
        return res;
    } else {
        uint32_t split = a->size / 2;
        BigInt* a0 = copyBigIntSubrange(a, 0, split);
        BigInt* a1 = copyBigIntSubrange(a, split, a->size - split);
        BigInt* b0 = copyBigIntSubrange(b, 0, split);
        BigInt* b1 = copyBigIntSubrange(b, split, b->size - split);
        BigInt* ac = absMulBigInt(a1, b1);
        BigInt* bd = absMulBigInt(a0, b0);
        absAddBigInt(&a1, a0, 0);
        absAddBigInt(&b1, b0, 0);
        freeBigInt(a0);
        freeBigInt(b0);
        BigInt* abcd = absMulBigInt(a1, b1);
        freeBigInt(a1);
        freeBigInt(b1);
        BigInt* acbd = copyBigInt(ac);
        absAddBigInt(&acbd, bd, 0);
        absSubBigInt(&abcd, acbd);
        freeBigInt(acbd);
        absAddBigInt(&bd, abcd, split);
        freeBigInt(abcd);
        absAddBigInt(&bd, ac, 2*split);
        freeBigInt(ac);
        return bd;
    }
}

static BigInt* absMulBigInt(BigInt* a, BigInt* b) {
    if (a->size == 0 || b->size == 0) {
        return createBigInt();
    } else if (a->size == 1 && b->size == 1) {
        BigInt* res = createBigIntCapacity(2);
        uint64_t tmp = (uint64_t)a->words[0] * (uint64_t)b->words[0];
        res->words[0] = tmp;
        res->words[1] = tmp >> WORD_SIZE;
        res->size = res->words[1] != 0 ? 2 : 1;
        return res;
    } else if (a->size < b->size) {
        return absMulBigIntBigSmall(b, a);
    } else {
        return absMulBigIntBigSmall(a, b);
    }
}


BigInt* mulBigInt(BigInt* a, BigInt* b) {
    if (b->size == 0 || b->size == 0) {
        return createBigInt();
    } else {
        BigInt* res = absMulBigInt(a, b);
        res->negative = a->negative != b->negative;
        return res;
    }
}

static uint32_t getWordInBigInt(BigInt* a, size_t i) {
    if (i < a->size) {
        return a->words[i];
    } else {
        return 0;
    }
}

static uint64_t getDWordInBigInt(BigInt* a, size_t i) {
    return (uint64_t)getWordInBigInt(a, i)
        | ((uint64_t)getWordInBigInt(a, i + 1) << WORD_SIZE);
}

static uint32_t guessMultiple(BigInt* a, BigInt* b) {
    int cmp = absCompareBigInt(a, b);
    if (cmp < 0) {
        return 0;
    } else if (cmp > 0) {
        uint64_t am = getDWordInBigInt(a, a->size - 2);
        uint64_t bm = getDWordInBigInt(b, a->size - 2);
        return am <= bm ? 1 : am / (bm + 1);
    } else {
        return 1;
    }
}

static void absDivRemBigInt(BigInt* a, BigInt* b, BigInt** div, BigInt** rem) {
    *div = copyBigInt(a);
    *rem = createBigIntCapacity(a->size);
    for (size_t i = a->size; i > 0;) {
        i--;
        for (size_t i = (*rem)->size; i > 0; i--) {
            (*rem)->words[i] = (*rem)->words[i - 1];
        }
        (*rem)->words[0] = a->words[i];
        (*rem)->size++;
        uint32_t word = 0;
        uint32_t guess;
        while ((guess = guessMultiple(*rem, b)) > 0) {
            word += guess;
            BigInt* bs = copyBigInt(b);
            absWordMulBigInt(&bs, guess);
            absSubBigInt(rem, bs);
            freeBigInt(bs);
        }
        (*div)->words[i] = word;
    }
    while ((*div)->size > 0 && (*div)->words[(*div)->size - 1] == 0) {
        (*div)->size--;
    }
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
        BigInt* div;
        BigInt* rem;
        absDivRemBigInt(a, b, &div, &rem);
        freeBigInt(rem);
        div->negative = a->negative != b->negative;
        return reallocIfNeeded(div, a->size);
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
        BigInt* res = createBigIntFromUnsigned(absWordRemBigInt(a, b->words[0]));
        res->negative = a->negative;
        return res;
    } else {
        BigInt* div;
        BigInt* rem;
        absDivRemBigInt(a, b, &div, &rem);
        freeBigInt(div);
        rem->negative = a->negative;
        return reallocIfNeeded(rem, a->size);
    }
}

typedef enum {
    BINARY_AND,
    BINARY_OR,
    BINARY_XOR,
} BinaryOperation;

static BigInt* binaryOpBigInt(BigInt* a, BigInt* b, BinaryOperation op) {
    if (a->size == 0 || b->size == 0) {
        return createBigInt();
    } else {
        uint32_t size = MAX(a->size, b->size) + 1;
        BigInt* res = createBigIntCapacity(size);
        switch (op) {
            case BINARY_AND:
                res->negative = a->negative && b->negative;
                break;
            case BINARY_OR:
                res->negative = a->negative || b->negative;
                break;
            case BINARY_XOR:
                res->negative = a->negative != b->negative;
                break;
        }
        uint32_t ac = a->negative ? 1 : 0;
        uint32_t bc = b->negative ? 1 : 0;
        uint32_t rc = res->negative ? 1 : 0;
        for (size_t i = 0; i < size; i++) {
            uint32_t aw = i < a->size ? a->words[i] : 0;
            if (a->negative) {
                aw = ~aw + ac;
                ac = aw == 0 ? 1 : 0;
            }
            uint32_t bw = i < b->size ? b->words[i] : 0;
            if (b->negative) {
                bw = ~bw + bc;
                bc = bw == 0 ? 1 : 0;
            }
            uint32_t rw;
            switch (op) {
                case BINARY_AND:
                    rw = aw & bw;
                    break;
                case BINARY_OR:
                    rw = aw | bw;
                    break;
                case BINARY_XOR:
                    rw = aw ^ bw;
                    break;
            }
            if (res->negative) {
                rw = ~rw + rc;
                rc = rw == 0 && rc == 1 ? 1 : 0;
            }
            res->words[i] = rw;
        }
        while (size > 0 && res->words[size - 1] == 0) {
            size--;
        }
        res->size = size;
        return res;
    }
}

BigInt* andBigInt(BigInt* a, BigInt* b) {
    return binaryOpBigInt(a, b, BINARY_AND);
}

BigInt* orBigInt(BigInt* a, BigInt* b) {
    return binaryOpBigInt(a, b, BINARY_OR);
}

BigInt* xorBigInt(BigInt* a, BigInt* b) {
    return binaryOpBigInt(a, b, BINARY_XOR);
}

BigInt* notBigInt(BigInt* a) {
    BigInt* res = negBigInt(a);
    if (res->negative) {
        absWordAddBigInt(&res, 1);
    } else {
        absWordSubBigInt(&res, 1);
    }
    return res;
}

BigInt* shiftLeftBigInt(BigInt* a, size_t r) {
    if (a->size == 0) {
        return createBigInt();
    } else if (r == 0) {
        return copyBigInt(a);
    } else {
        uint32_t words = r / WORD_SIZE;
        uint32_t bits = r % WORD_SIZE;
        BigInt* res = createBigIntCapacity(a->size + words + 1);
        memset(res->words, 0, (a->size + words + 1) * sizeof(uint32_t));
        for (size_t i = 0; i < a->size; i++) {
            res->words[words + i] |= a->words[i] << bits;
            if (bits != 0) {
                res->words[words + i + 1] |= a->words[i] >> (WORD_SIZE - bits);
            }
        }
        res->negative = a->negative;
        if (res->words[a->size + words] == 0) {
            res->size = a->size + words;
        } else {
            res->size = a->size + words + 1;
        }
        return res;
    }
}

BigInt* shiftRightBigInt(BigInt* a, size_t r) {
    if (a->size < r / WORD_SIZE) {
        return a->negative ? createBigIntFrom(-1) : createBigInt();
    } else if (r == 0) {
        return copyBigInt(a);
    } else {
        uint32_t words = r / WORD_SIZE;
        uint32_t bits = r % WORD_SIZE;
        BigInt* res = createBigIntCapacity(a->size - words);
        memset(res->words, 0, (a->size - words) * sizeof(uint32_t));
        for (size_t i = words; i < a->size; i++) {
            res->words[i - words] |= a->words[i] >> bits;
            if (bits != 0 && i > words) {
                res->words[i - words - 1] |= a->words[i] << (WORD_SIZE - bits);
            }
        }
        res->negative = a->negative;
        if (res->words[a->size - words - 1] == 0) {
            res->size = a->size - words - 1;
        } else {
            res->size = a->size - words;
        }
        return res;
    }
}

String stringForBigInt(BigInt* bi, int base) {
    if (isBigIntZero(bi)) {
        return copyFromCString("0");
    } else {
        StringBuilder builder;
        initStringBuilder(&builder);
        BigInt* copy = absBigInt(bi);
        while (!isBigIntZero(copy)) {
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

intmax_t intMaxForBigInt(BigInt* bi) {
    return (intmax_t)uintMaxForBigInt(bi);
}

uintmax_t uintMaxForBigInt(BigInt* bi) {
    size_t size = (sizeof(uintmax_t) + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    uintmax_t res = 0;
    for (size_t i = 0; i < size && i < bi->size; i++) {
        res |= (uintmax_t)bi->words[i] << (i * WORD_SIZE);
    }
    if (bi->negative) {
        return -res;
    } else {
        return res;
    }
}

double doubleForBigInt(BigInt* fi) {
    double res = 0;
    for (size_t i = fi->size; i > 0;) {
        i--,
        res *= (1UL << WORD_SIZE);
        res += fi->words[i];
    }
    if (fi->negative) {
        return -res;
    } else {
        return res;
    }
}

