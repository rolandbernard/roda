#ifndef _ANALYSIS_VARBUILD_H_
#define _ANALYSIS_VARBUILD_H_

#include "ast/ast.h"
#include "errors/msgcontext.h"

void buildSymbolTables(MessageContext* context, AstNode* root);

#endif
