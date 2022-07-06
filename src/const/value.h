#ifndef _RODA_CONST_VALUE_H_
#define _RODA_CONST_VALUE_H_

#include <stdint.h>

#include "types/types.h"

typedef struct {
    Type* type;
    union {
        intmax_t sint;
        uintmax_t uint;
        float f32;
        double f64;
        bool boolean;
    };
} ConstValue;

#endif
