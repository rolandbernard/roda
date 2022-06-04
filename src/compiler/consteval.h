#ifndef _COMPILER_CONSTEVAL_H_
#define _COMPILER_CONSTEVAL_H_

#include <stdint.h>

#include "ast/ast.h"
#include "compiler/context.h"

typedef enum {
    CONST_INT,
    CONST_UINT,
    CONST_REAL,
} ConstValueKind;

typedef struct {
    ConstValueKind kind;
    union {
        intmax_t sint;
        uintmax_t uint;
        double real;
    };
} ConstValue;

ConstValue evaluateConstExpr(CompilerContext* context, AstNode* node);

#endif
