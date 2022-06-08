
#include "ast/ast.h"
#include "util/alloc.h"

#include "ast/astlist.h"

#define INITIAL_LIST_CAPACITY 32

DynamicAstList* createDynamicAstList() {
    DynamicAstList* ret = ALLOC(DynamicAstList, 1);
    initDynamicAstList(ret, 0);
    return ret;
}

DynamicAstList* createDynamicAstListOfSize(size_t size) {
    DynamicAstList* ret = ALLOC(DynamicAstList, 1);
    initDynamicAstList(ret, size);
    return ret;
}

void initDynamicAstList(DynamicAstList* list, size_t size) {
    initAstNode((AstNode*)list, AST_LIST, invalidSpan());
    list->nodes = ALLOC(AstNode*, size);
    list->count = 0;
    list->capacity = size;
}

void deinitDynamicAstList(DynamicAstList* list) {
    for (size_t i = 0; i < list->count; i++) {
        freeAstNode(list->nodes[i]);
    }
    FREE(list->nodes);
}

void freeDynamicAstList(DynamicAstList* list) {
    deinitDynamicAstList(list);
    FREE(list);
}

void addToDynamicAstList(DynamicAstList* list, AstNode* node) {
    if (list->count == list->capacity) {
        if (list->capacity == 0) {
            list->capacity = INITIAL_LIST_CAPACITY;
        } else {
            list->capacity *= 2;
        }
        list->nodes = REALLOC(AstNode*, list->nodes, list->capacity);
    }
    list->nodes[list->count] = node;
    list->count++;
}

void resizeDynamicAstList(DynamicAstList* list, size_t size) {
    list->nodes = REALLOC(AstNode*, list->nodes, size);
    if (list->count < size) {
        list->count = size;
    }
    list->capacity = size;
}

void fitDynamicAstList(DynamicAstList* list) {
    resizeDynamicAstList(list, list->count);
}

AstList* toStaticAstList(DynamicAstList* list) {
    fitDynamicAstList(list);
    return REALLOC(AstList, list, 1);
}

DynamicAstList* toDynamicAstList(AstList* list) {
    DynamicAstList* ret = REALLOC(DynamicAstList, list, 1);
    ret->capacity = ret->count;
    return ret;
}

