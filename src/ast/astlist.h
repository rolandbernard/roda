#ifndef _RODA_AST_ASTLIST_H_
#define _RODA_AST_ASTLIST_H_

#include "ast/ast.h"

AstList* createEmptyAstList();

AstList* createAstListOfSize(size_t size);

void addToAstList(AstList* list, AstNode* node);

void resizeAstList(AstList* list, size_t size);

void fitAstList(AstList* list);

#endif
