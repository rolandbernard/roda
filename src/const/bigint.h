#ifndef _RODA_CONST_BIGINT_H_
#define _RODA_CONST_BIGINT_H_

#include <stdint.h>
#include <stdbool.h>

#include "text/string.h"

typedef struct {
    bool negative;
    uint32_t size;
    uint32_t words[];
} BigInt;

BigInt* createBigInt();

BigInt* createBigIntFrom(intmax_t value);

BigInt* createBigIntFromString(ConstString str, int base);

BigInt* copyBigInt(BigInt* bi);

void freeBigInt(BigInt* bi);

int signOfBigInt(BigInt* bi);

bool isBigIntZero(BigInt* a);

int compareBigInt(BigInt* a, BigInt* b);

BigInt* negBigInt(BigInt* bi);

BigInt* absBigInt(BigInt* bi);

BigInt* addBigInt(BigInt* a, BigInt* b);

BigInt* subBigInt(BigInt* a, BigInt* b);

BigInt* mulBigInt(BigInt* a, BigInt* b);

BigInt* divBigInt(BigInt* a, BigInt* b);

BigInt* remBigInt(BigInt* a, BigInt* b);

BigInt* andBigInt(BigInt* a, BigInt* b);

BigInt* orBigInt(BigInt* a, BigInt* b);

BigInt* xorBigInt(BigInt* a, BigInt* b);

BigInt* notBigInt(BigInt* a);

BigInt* shiftLeftBigInt(BigInt* a, size_t r);

BigInt* shiftRightBigInt(BigInt* a, size_t r);

String stringForBigInt(BigInt* bi, int base);

intmax_t intMaxForBigInt(BigInt* bi);

#endif
