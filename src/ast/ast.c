
#include <string.h>
#include <inttypes.h>

#include "util/alloc.h"

#include "ast/ast.h"

AstBinary* createAstBinary(AstNodeKind kind, AstNode* left, AstNode* right) {
    AstBinary* node = NEW(AstBinary);
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

AstUnary* createAstUnary(AstNodeKind kind, AstNode* operand) {
    AstUnary* node = NEW(AstUnary);
    node->kind = kind;
    node->op = operand;
    return node;
}

AstList* createAstList(AstNodeKind kind, size_t count, AstNode** nodes) {
    AstList* node = NEW(AstList);
    node->kind = kind;
    node->count = count;
    node->nodes = nodes;
    return node;
}

AstIfElse* createAstIfElse(AstNode* cond, AstNode* if_block, AstNode* else_block) {
    AstIfElse* node = NEW(AstIfElse);
    node->condition = cond; 
    node->if_block = if_block;
    node->else_block = else_block;
    return node;
}

AstVar* createAstVar(const char* name) {
    AstVar* node = NEW(AstVar);
    node->kind = AST_VAR;
    node->name = strdup(name);
    return node;
}

AstVarDef* createAstVarDef(AstNode* dst, AstNode* type, AstNode* val) {
    AstVarDef* node = NEW(AstVarDef);
    node->kind = AST_VARDEF;
    node->dst = dst;
    node->type = type;
    node->val = val;
    return node;
}

AstWhile* createAstWhile(AstNode* cond, AstNode* block) {
    AstWhile* node = NEW(AstWhile);
    node->kind = AST_WHILE;
    node->condition = cond;
    node->block = block;
    return node;
}

AstFn* createAstFn(const char* name, AstList* arguments, AstNode* ret_type, AstNode* body) {
    AstFn* node = NEW(AstFn);
    node->name = strdup(name);
    node->arguments = arguments;
    node->ret_type = ret_type;
    node->body = body;
    return node;
}

AstCall* createAstCall(AstNode* func, AstList* arguments) {
    AstCall* node = NEW(AstCall);
    node->kind = AST_CALL;
    node->function = func;
    node->arguments = arguments;
    return node;
}

AstInt* createAstInt(const char* string) {
    AstInt* node = NEW(AstInt);
    node->kind = AST_INT;
    node->number = strtoimax(string, NULL, 10);
    return node;
}

AstReal* createAstReal(const char* string) {
    AstReal* node = NEW(AstReal);
    node->kind = AST_REAL;
    node->number = strtod(string, NULL);
    return node;
}

AstStr* createAstStr(const char* string) {
    AstStr* node = NEW(AstStr);
    node->kind = AST_STR;
    node->string = strdup(string);
    return node;
}

void freeAstNode(AstNode* node) {
    FREE(node);
}

