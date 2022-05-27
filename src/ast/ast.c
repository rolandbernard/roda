
#include <inttypes.h>

#include "util/alloc.h"
#include "util/text.h"

#include "ast/ast.h"

#define SET_PARENT(NODE) if (NODE != NULL) { NODE->parent = (AstNode*)node;}

AstBinary* createAstBinary(AstNodeKind kind, AstNode* left, AstNode* right) {
    AstBinary* node = NEW(AstBinary);
    node->kind = kind;
    node->left = left;
    node->right = right;
    SET_PARENT(left);
    SET_PARENT(right);
    return node;
}

AstUnary* createAstUnary(AstNodeKind kind, AstNode* operand) {
    AstUnary* node = NEW(AstUnary);
    node->kind = kind;
    node->op = operand;
    SET_PARENT(operand);
    return node;
}

AstList* createAstList(AstNodeKind kind, size_t count, AstNode** nodes) {
    AstList* node = NEW(AstList);
    node->kind = kind;
    node->count = count;
    node->nodes = nodes;
    for (size_t i = 0; i < count; i++) {
        SET_PARENT(nodes[i]);
    }
    return node;
}

AstIfElse* createAstIfElse(AstNode* cond, AstNode* if_block, AstNode* else_block) {
    AstIfElse* node = NEW(AstIfElse);
    node->kind = AST_IF_ELSE;
    node->condition = cond; 
    node->if_block = if_block;
    node->else_block = else_block;
    SET_PARENT(cond);
    SET_PARENT(if_block);
    SET_PARENT(else_block);
    return node;
}

AstVar* createAstVar(char* name) {
    AstVar* node = NEW(AstVar);
    node->kind = AST_VAR;
    node->name = name;
    return node;
}

AstVarDef* createAstVarDef(AstNode* dst, AstNode* type, AstNode* val) {
    AstVarDef* node = NEW(AstVarDef);
    node->kind = AST_VARDEF;
    node->dst = dst;
    node->type = type;
    node->val = val;
    SET_PARENT(dst);
    SET_PARENT(type);
    SET_PARENT(val);
    return node;
}

AstWhile* createAstWhile(AstNode* cond, AstNode* block) {
    AstWhile* node = NEW(AstWhile);
    node->kind = AST_WHILE;
    node->condition = cond;
    node->block = block;
    SET_PARENT(cond);
    SET_PARENT(block);
    return node;
}

AstFn* createAstFn(char* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags) {
    AstFn* node = NEW(AstFn);
    node->kind = AST_FN;
    node->name = name;
    node->arguments = arguments;
    node->ret_type = ret_type;
    node->body = body;
    node->flags = flags;
    SET_PARENT(arguments);
    SET_PARENT(ret_type);
    SET_PARENT(body);
    return node;
}

AstCall* createAstCall(AstNode* func, AstList* arguments) {
    AstCall* node = NEW(AstCall);
    node->kind = AST_CALL;
    node->function = func;
    node->arguments = arguments;
    SET_PARENT(func);
    SET_PARENT(arguments);
    return node;
}

AstInt* createAstInt(char* string) {
    AstInt* node = NEW(AstInt);
    node->kind = AST_INT;
    node->number = strtoimax(string, NULL, 10);
    FREE(string);
    return node;
}

AstReal* createAstReal(char* string) {
    AstReal* node = NEW(AstReal);
    node->kind = AST_REAL;
    node->number = strtod(string, NULL);
    FREE(string);
    return node;
}

AstStr* createAstStr(char* string) {
    AstStr* node = NEW(AstStr);
    node->kind = AST_STR;
    inlineDecodeStringLiteral(string);
    node->string = string;
    return node;
}

AstTypeDef* createAstTypeDef(char* name, AstNode* value) {
    AstTypeDef* node = NEW(AstTypeDef);
    node->kind = AST_TYPEDEF;
    node->name = name;
    node->value = value;
    SET_PARENT(value);
    return node;
}

AstArgDef* createAstArgDef(char* name, AstNode* type) {
    AstArgDef* node = NEW(AstArgDef);
    node->kind = AST_ARGDEF;
    node->name = name;
    node->type = type;
    SET_PARENT(type);
    return node;
}

void freeAstNode(AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ASSIGN:
            case AST_INDEX:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_OR:
            case AST_AND:
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT:
            case AST_ARRAY:
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                freeAstNode(n->left);
                freeAstNode(n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                freeAstNode(n->op);
                break;
            }
            case AST_LIST:
            case AST_ROOT:
            case AST_BLOCK: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    freeAstNode(n->nodes[i]);
                }
                FREE(n->nodes);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                FREE(n->name);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                freeAstNode(n->dst);
                freeAstNode(n->type);
                freeAstNode(n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                freeAstNode(n->condition);
                freeAstNode(n->if_block);
                freeAstNode(n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                freeAstNode(n->condition);
                freeAstNode(n->block);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                FREE(n->name);
                freeAstNode((AstNode*)n->arguments);
                freeAstNode(n->ret_type);
                freeAstNode(n->body);
                break;

            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                freeAstNode(n->function);
                freeAstNode((AstNode*)n->arguments);
                break;
            }
            case AST_STR: {
                AstStr* n = (AstStr*)node;
                FREE(n->string);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                FREE(n->name);
                freeAstNode(n->value);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                FREE(n->name);
                freeAstNode(n->type);
                break;
            }
            case AST_INT:
            case AST_REAL: break;
        }
        FREE(node);
    }
}

