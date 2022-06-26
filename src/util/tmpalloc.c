
#include "util/alloc.h"

#include "util/tmpalloc.h"

#define MINIMUM_BUFFER_CAPACITY 100UL

void initTempAlloc(TempAlloc* alloc) {
    for (size_t i = 0; i < TEMP_BUFFER_SIZES; i++) {
        alloc->pages[i] = NULL;
    }
}

void deinitTempAlloc(TempAlloc* alloc) {
    TempBuffer* cur = alloc->pages[0];
    while (cur != NULL) {
        TempBuffer* next = cur->next;
        FREE(cur);
        cur = next;
    }
}

static size_t findBufferCapacityBucket(size_t size) {
    size_t i = 0;
    size_t cap = MINIMUM_BUFFER_CAPACITY;
    while (size > cap && i < TEMP_BUFFER_SIZES) {
        cap *= 2;
        i++;
    }
    return i;
}

static TempBuffer* findBufferWithCapacity(TempAlloc* alloc, size_t size) {
    size_t bucket = findBufferCapacityBucket(size);
    TempBuffer* cur = alloc->pages[bucket];
    while (cur != NULL && cur->capacity - cur->used < size) {
        cur = cur->next;
    }
    if (cur != NULL) {
        return cur;
    } else {
        if ((MINIMUM_BUFFER_CAPACITY << bucket) > size) {
            size = MINIMUM_BUFFER_CAPACITY << bucket;
        }
        TempBuffer* buffer = checkedAlloc(sizeof(TempAlloc) + size);
        buffer->next = alloc->pages[bucket];
        buffer->used = 0;
        buffer->count = 0;
        buffer->capacity = size;
        for (size_t i = bucket + 1; i > 0;) {
            i--;
            if (alloc->pages[i] == buffer->next) {
                alloc->pages[i] = buffer;
            } else if (alloc->pages[i]->next == buffer->next) {
                alloc->pages[i]->next = buffer;
            } else {
                break;
            }
        }
        return buffer;
    }
}

void* allocTempBuffer(TempAlloc* alloc, size_t size) {
    TempBuffer* buffer = findBufferWithCapacity(alloc, size);
    void* ptr = buffer->data + buffer->used;
    buffer->used += size;
    buffer->count++;
    return ptr;
}

void freeTempBuffer(TempAlloc* alloc, void* ptr) {
    TempBuffer* cur = alloc->pages[0];
    while (cur != NULL) {
        if ((void*)cur->data <= ptr && (void*)(cur->data + cur->capacity) > ptr) {
            cur->count--;
            if (cur->count == 0) {
                cur->used = 0;
            }
            break;
        }
        cur = cur->next;
    }
}

