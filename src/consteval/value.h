#ifndef _RODA_CONSTEVAL_VALUE_H_
#define _RODA_CONSTEVAL_VALUE_H_

#include <stdint.h>

#include "compiler/types.h"

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
