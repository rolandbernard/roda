
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "util/alloc.h"

#include "errors/fatalerror.h"

void* checkedAlloc(size_t size) {
    void* ret = malloc(size);
    if (ret == NULL) {
        fatalError(createFromConstCString(strerror(ENOMEM)));
        exit(EXIT_FAILURE);
    } else {
        return ret;
    }
}

void* checkedRealloc(void* original, size_t size) {
    void* ret = realloc(original, size);
    if (ret == NULL) {
        fatalError(createFromConstCString(strerror(ENOMEM)));
        exit(EXIT_FAILURE);
    } else {
        return ret;
    }
}

void* checkedCalloc(size_t n, size_t size) {
    void* ret = calloc(n, size);
    if (ret == NULL) {
        fatalError(createFromConstCString(strerror(ENOMEM)));
        exit(EXIT_FAILURE);
    } else {
        return ret;
    }
}
