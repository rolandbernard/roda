#ifndef _RODA_AST_ASTLIST_H_
#define _RODA_AST_ASTLIST_H_

#include "ast/ast.h"

typedef struct {
    AST_NODE_BASE
    AstNode** nodes;
    size_t count;
    size_t capacity;
} DynamicAstList;

DynamicAstList* createDynamicAstList();

DynamicAstList* createDynamicAstListOfSize(size_t size);

void initDynamicAstList(DynamicAstList* list, size_t size);

void deinitDynamicAstList(DynamicAstList* list);

void freeDynamicAstList(DynamicAstList* list);

void addToDynamicAstList(DynamicAstList* list, AstNode* node);

void resizeDynamicAstList(DynamicAstList* list, size_t size);

void fitDynamicAstList(DynamicAstList* list);

AstList* toStaticAstList(DynamicAstList* list);

DynamicAstList* toDynamicAstList(AstList* list);

#endif
