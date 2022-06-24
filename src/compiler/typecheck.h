#ifndef _RODA_COMPILER_TYPECHECK_H_
#define _RODA_COMPILER_TYPECHECK_H_

#include "ast/ast.h"
#include "compiler/context.h"

void typeCheckExpr(CompilerContext* context, AstNode* node);

void checkTypeConstraints(CompilerContext* context, AstNode* node);

void runTypeChecking(CompilerContext* context);

#endif
