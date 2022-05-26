#ifndef _UTIL_ALLOC_H_
#define _UTIL_ALLOC_H_

#include <stdlib.h>

#define NEW(TYPE) ALLOC(TYPE, 1)

#define ALLOC(TYPE, COUNT) ((TYPE*)malloc(sizeof(TYPE) * (COUNT)))

#define REALLOC(TYPE, OLD, COUNT) ((TYPE*)realloc(OLD, sizeof(TYPE) * (COUNT)))

#define ZALLOC(TYPE, COUNT) ((TYPE*)calloc(COUNT, sizeof(TYPE)))

#define FREE(PTR) free(PTR)

#endif
