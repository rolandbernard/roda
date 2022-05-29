#ifndef _ANALYSIS_VARBUILD_H_
#define _ANALYSIS_VARBUILD_H_

#include "ast/ast.h"
#include "compiler/context.h"
#include "errors/msgcontext.h"

void buildSymbolTables(CompilerContext* context, AstNode* root);

#endif
