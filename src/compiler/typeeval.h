#ifndef _COMPILER_TYPEEVAL_H_
#define _COMPILER_TYPEEVAL_H_

#include "ast/ast.h"
#include "compiler/types.h"
#include "compiler/context.h"

const Type* evaluateTypeExpr(CompilerContext* context, AstNode* node);

#endif
