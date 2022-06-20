#ifndef _RODA_COMPILER_CONSTEVAL_H_
#define _RODA_COMPILER_CONSTEVAL_H_

#include <stdint.h>

#include "ast/ast.h"
#include "compiler/context.h"

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

ConstValue createConstError(CompilerContext* context);

ConstValue createConstInt(CompilerContext* context, size_t size, intmax_t value);

ConstValue createConstUInt(CompilerContext* context, size_t size, uintmax_t value);

ConstValue createConstF32(CompilerContext* context, float value);

ConstValue createConstF64(CompilerContext* context, double value);

ConstValue createConstBool(CompilerContext* context, bool value);

ConstValue evaluateConstExpr(CompilerContext* context, AstNode* node);

#endif
