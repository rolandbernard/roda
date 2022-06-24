
#include "ast/ast.h"
#include "util/alloc.h"

#include "ast/astlist.h"

#define INITIAL_LIST_CAPACITY 32

static void initAstList(AstList* list, size_t size) {
    initAstNode((AstNode*)list, AST_LIST, invalidSpan());
    list->nodes = ALLOC(AstNode*, size);
    list->count = 0;
    list->capacity = size;
}

AstList* createEmptyAstList() {
    AstList* ret = ALLOC(AstList, 1);
    initAstList(ret, 0);
    return ret;
}

AstList* createAstListOfSize(size_t size) {
    AstList* ret = ALLOC(AstList, 1);
    initAstList(ret, size);
    return ret;
}

void addToAstList(AstList* list, AstNode* node) {
    if (list->count == list->capacity) {
        if (list->capacity == 0) {
            list->capacity = INITIAL_LIST_CAPACITY;
        } else {
            list->capacity *= 2;
        }
        list->nodes = REALLOC(AstNode*, list->nodes, list->capacity);
    }
    list->nodes[list->count] = node;
    node->parent = (AstNode*)list;
    node->parent_idx = list->count;
    list->count++;
}

void resizeAstList(AstList* list, size_t size) {
    list->nodes = REALLOC(AstNode*, list->nodes, size);
    if (list->count < size) {
        list->count = size;
    }
    list->capacity = size;
}

void fitAstList(AstList* list) {
    resizeAstList(list, list->count);
}

