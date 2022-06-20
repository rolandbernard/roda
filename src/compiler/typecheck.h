#ifndef _RODA_COMPILER_TYPECHECK_H_
#define _RODA_COMPILER_TYPECHECK_H_

#include "ast/ast.h"
#include "compiler/context.h"

void typeCheckConstExpr(CompilerContext* context, AstNode* node, Type* assumption);

void runTypeChecking(CompilerContext* context);

#endif
