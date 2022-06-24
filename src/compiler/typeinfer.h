#ifndef _RODA_COMPILER_TYPEINFER_H_
#define _RODA_COMPILER_TYPEINFER_H_

#include "ast/ast.h"
#include "compiler/context.h"

void typeInferExpr(CompilerContext* context, AstNode* node, Type* assumption);

void runTypeInference(CompilerContext* context);

#endif
