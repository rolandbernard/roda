#ifndef _UTIL_SORT_H_
#define _UTIL_SORT_H_

#include <stddef.h>
#include <stdbool.h>

typedef void (*SwapFunction)(size_t idx_a, size_t idx_b, void* ctx);
typedef bool (*LessOrEqualFunction)(size_t idx_a, size_t idx_b, void* ctx);

void swap(void* array, size_t el_size, size_t i, size_t j);

void heapify(size_t i, size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt);

void buildHeap(size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt);

void heapSort(size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt);

#endif
