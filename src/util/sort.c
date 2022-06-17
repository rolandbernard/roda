
#include <string.h>

#include "util/sort.h"

void swap(void* array, size_t el_size, size_t i, size_t j) {
    char tmp[el_size];
    memcpy(tmp, ((char*)array) + i * el_size, el_size);
    memcpy(((char*)array) + i * el_size, ((char*)array) + j * el_size, el_size);
    memcpy(((char*)array) + j * el_size, tmp, el_size);
}

void heapify(size_t i, size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt) {
    size_t left = 2 * (i + 1) - 1;
    size_t right = 2 * (i + 1);
    size_t max = i;
    if (left < size && !cmp(left, max, cxt)) {
        max = left;
    }
    if (right < size && !cmp(right, max, cxt)) {
        max = right;
    }
    if (max != i) {
        swap(max, i, cxt);
        heapify(max, size, swap, cmp, cxt);
    }
}

void buildHeap(size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt) {
    for (size_t i = size / 2; i > 0;) {
        i--;
        heapify(i, size, swap, cmp, cxt);
    }
}

void heapSort(size_t size, SwapFunction swap, LessOrEqualFunction cmp, void* cxt) {
    buildHeap(size, swap, cmp, cxt);
    for (size_t i = 0; i < size; i++) {
        swap(0, size - 1 - i, cxt);
        heapify(0, size - 1 - i, swap, cmp, cxt);
    }
}
