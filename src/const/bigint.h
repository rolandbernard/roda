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

BigInt* createBigIntFromString(ConstString str, uint32_t base);

BigInt* copyBigInt(BigInt* bi);

void freeBigInt(BigInt* bi);

int signOfBigInt(BigInt* bi);

int compareBigInt(BigInt* a, BigInt* b);

BigInt* negBigInt(BigInt* bi);

BigInt* addBigInt(BigInt* a, BigInt* b);

BigInt* subBigInt(BigInt* a, BigInt* b);

BigInt* mulBigInt(BigInt* a, BigInt* b);

BigInt* divBigInt(BigInt* a, BigInt* b);

BigInt* remBigInt(BigInt* a, BigInt* b);

BigInt* andBigInt(BigInt* a, BigInt* b);

BigInt* orBigInt(BigInt* a, BigInt* b);

BigInt* xorBigInt(BigInt* a, BigInt* b);

String stringForBigInt(BigInt* bi, uint32_t base);

#endif
