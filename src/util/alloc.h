#ifndef _RODA_UTIL_ALLOC_H_
#define _RODA_UTIL_ALLOC_H_

#include <stdlib.h>

void* checkedAlloc(size_t size);

void* checkedRealloc(void* original, size_t size);

void* checkedCalloc(size_t n, size_t size);

#define NEW(TYPE) ALLOC(TYPE, 1)

#define ALLOC(TYPE, COUNT) (checkedAlloc(sizeof(TYPE) * (COUNT)))

#define REALLOC(TYPE, OLD, COUNT) (checkedRealloc(OLD, sizeof(TYPE) * (COUNT)))

#define ZALLOC(TYPE, COUNT) (checkedCalloc(COUNT, sizeof(TYPE)))

#define FREE(PTR) free(PTR)

#endif
