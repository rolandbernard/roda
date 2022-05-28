
#include "util/hash.h"
#include <stdint.h>

#define BYTES_HASH_START_PRIME 7919
#define BYTES_HASH_MUL_PRIME   293

size_t hashBytes(const void* mem, size_t size) {
    size_t hash = BYTES_HASH_START_PRIME;
    while (size > 0) {
        hash *= BYTES_HASH_MUL_PRIME;
        hash += *(const char*)mem;
        mem++;
        size--;
    }
    return hashInt(hash);
}

size_t hashInt(size_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

size_t hashCString(const char* s) {
    size_t hash = BYTES_HASH_START_PRIME;
    while (*s != 0) {
        hash *= BYTES_HASH_MUL_PRIME;
        hash += *s;
        s++;
    }
    return hashInt(hash);
}

size_t hashCombine(size_t first, size_t second) {
    return first ^ (second + 0x9e3779b9 + (first << 6) + (first >> 2));
}

size_t hashString(ConstString str) {
    return hashBytes(str.data, str.length);
}

