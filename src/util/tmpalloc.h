#ifndef _RODA_UTIL_TMPALLOC_H_
#define _RODA_UTIL_TMPALLOC_H_

#include <stddef.h>

typedef struct TempBuffer {
    struct TempBuffer* next;
    size_t count;
    size_t capacity;
    size_t used;
    char data[];
} TempBuffer;

#define TEMP_BUFFER_SIZES 10

typedef struct {
    TempBuffer* pages[TEMP_BUFFER_SIZES];
} TempAlloc;

void initTempAlloc(TempAlloc* alloc);

void deinitTempAlloc(TempAlloc* alloc);

void* allocTempBuffer(TempAlloc* alloc, size_t size);

void freeTempBuffer(TempAlloc* alloc, void* ptr);

#endif
