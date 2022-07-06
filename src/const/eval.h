#ifndef _RODA_CONST_EVAL_H_
#define _RODA_CONST_EVAL_H_

#include "ast/ast.h"
#include "compiler/context.h"
#include "const/value.h"

ConstValue createConstError(CompilerContext* context);

ConstValue createConstInt(CompilerContext* context, size_t size, intmax_t value);

ConstValue createConstUInt(CompilerContext* context, size_t size, uintmax_t value);

ConstValue createConstF32(CompilerContext* context, float value);

ConstValue createConstF64(CompilerContext* context, double value);

ConstValue createConstBool(CompilerContext* context, bool value);

ConstValue evaluateConstExpr(CompilerContext* context, AstNode* node);

bool checkValidInConstExpr(CompilerContext* context, AstNode* node);

void evaluateConstantDefinition(CompilerContext* context, AstVarDef* def);

#endif
