#ifndef _COMPILER_TYPEEVAL_H_
#define _COMPILER_TYPEEVAL_H_

#include "ast/ast.h"
#include "compiler/types.h"
#include "compiler/context.h"

Type* evaluateTypeExpr(CompilerContext* context, AstNode* node);

void sortStructFieldsByName(AstList* n);

bool checkStructFieldsHaveNoDups(CompilerContext* context, AstList* n);

#endif
