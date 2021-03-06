#ifndef _RODA_COMPILER_TYPEEVAL_H_
#define _RODA_COMPILER_TYPEEVAL_H_

#include "ast/ast.h"
#include "compiler/context.h"
#include "types/types.h"

Type* evaluateTypeExpr(CompilerContext* context, AstNode* node);

void sortStructFieldsByName(AstList* n);

bool checkStructFieldsHaveNoDups(CompilerContext* context, AstList* n);

#endif
