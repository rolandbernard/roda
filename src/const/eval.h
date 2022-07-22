#ifndef _RODA_CONST_EVAL_H_
#define _RODA_CONST_EVAL_H_

#include "ast/ast.h"
#include "compiler/context.h"
#include "const/value.h"

ConstValue* evaluateConstExpr(CompilerContext* context, AstNode* node);

bool checkValidInConstExpr(CompilerContext* context, AstNode* node);

void evaluateConstantDefinition(CompilerContext* context, AstVarDef* def);

#endif
