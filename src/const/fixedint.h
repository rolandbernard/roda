#ifndef _RODA_CONST_FIXEDINT_H_
#define _RODA_CONST_FIXEDINT_H_

#include <stdint.h>
#include <stdbool.h>

#include "text/string.h"
#include "const/bigint.h"

typedef struct {
    uint32_t size;
    uint32_t words[];
} FixedInt;

FixedInt* createFixedInt(uint32_t size);

FixedInt* createFixedIntFrom(uint32_t size, intmax_t value);

FixedInt* createFixedIntFromUnsigned(uint32_t size, uintmax_t value);

FixedInt* createFixedIntFromDouble(uint32_t size, double value);

FixedInt* createFixedIntFromString(uint32_t size, ConstString str, int base);

FixedInt* createFixedIntFromBigInt(uint32_t size, BigInt* bi);

BigInt* createBigIntFromFixedIntSignExtend(FixedInt* fi);

BigInt* createBigIntFromFixedIntZeroExtend(FixedInt* fi);

FixedInt* resizeFixedIntSignExtend(FixedInt* fi, uint32_t size);

FixedInt* resizeFixedIntZeroExtend(FixedInt* fi, uint32_t size);

FixedInt* copyFixedInt(FixedInt* fi);

void freeFixedInt(FixedInt* fi);

int signOfFixedInt(FixedInt* fi);

bool isFixedIntZero(FixedInt* a);

int compareFixedIntSigned(FixedInt* a, FixedInt* b);

int compareFixedIntUnsigned(FixedInt* a, FixedInt* b);

FixedInt* negFixedInt(FixedInt* fi);

FixedInt* absFixedInt(FixedInt* fi);

FixedInt* addFixedInt(FixedInt* a, FixedInt* b);

FixedInt* subFixedInt(FixedInt* a, FixedInt* b);

FixedInt* mulFixedInt(FixedInt* a, FixedInt* b);

FixedInt* udivFixedInt(FixedInt* a, FixedInt* b);

FixedInt* sdivFixedInt(FixedInt* a, FixedInt* b);

FixedInt* uremFixedInt(FixedInt* a, FixedInt* b);

FixedInt* sremFixedInt(FixedInt* a, FixedInt* b);

FixedInt* andFixedInt(FixedInt* a, FixedInt* b);

FixedInt* orFixedInt(FixedInt* a, FixedInt* b);

FixedInt* xorFixedInt(FixedInt* a, FixedInt* b);

FixedInt* notFixedInt(FixedInt* a);

FixedInt* shiftLeftFixedInt(FixedInt* a, size_t r);

FixedInt* shiftRightLogicalFixedInt(FixedInt* a, size_t r);

FixedInt* shiftRightArithmeticFixedInt(FixedInt* a, size_t r);

String stringForFixedIntUnsigned(FixedInt* fi, int base);

String stringForFixedIntSigned(FixedInt* fi, int base);

intmax_t intMaxForFixedInt(FixedInt* fi);

uintmax_t uintMaxForFixedInt(FixedInt* fi);

double doubleForFixedIntUnsigned(FixedInt* fi);

double doubleForFixedIntSigned(FixedInt* fi);

uint64_t* convertTo64BitWordsZeroExtend(FixedInt* a, size_t* length);

uint64_t* convertTo64BitWordsSignExtend(FixedInt* a, size_t* length);

#endif
