#ifndef _UTIL_HASH_H_
#define _UTIL_HASH_H_

#include "text/string.h"

size_t hashBytes(const char* mem, size_t size);

size_t hashInt(size_t value);

size_t hashCString(const char* s);

size_t hashCombine(size_t first, size_t second);

size_t hashString(ConstString str);

#endif
