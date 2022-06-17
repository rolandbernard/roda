
#include <inttypes.h>

#include "util/alloc.h"

#include "ast/ast.h"

#define SET_PARENT(NODE) if (NODE != NULL) { NODE->parent = (AstNode*)node;}

void initAstNode(AstNode* node, AstNodeKind kind, Span loc) {
    node->kind = kind;
    node->parent = NULL;
    node->location = loc;
    node->res_type = NULL;
    node->res_type_reasoning = NULL;
}

AstBinary* createAstBinary(Span loc, AstNodeKind kind, AstNode* left, AstNode* right) {
    AstBinary* node = NEW(AstBinary);
    initAstNode((AstNode*)node, kind, loc);
    node->left = left;
    node->right = right;
    SET_PARENT(left);
    SET_PARENT(right);
    return node;
}

AstUnary* createAstUnary(Span loc, AstNodeKind kind, AstNode* operand) {
    AstUnary* node = NEW(AstUnary);
    initAstNode((AstNode*)node, kind, loc);
    node->op = operand;
    SET_PARENT(operand);
    return node;
}

AstList* createAstList(Span loc, AstNodeKind kind, size_t count, AstNode** nodes) {
    AstList* node = NEW(AstList);
    initAstNode((AstNode*)node, kind, loc);
    node->count = count;
    node->nodes = nodes;
    for (size_t i = 0; i < count; i++) {
        SET_PARENT(nodes[i]);
    }
    return node;
}

AstIfElse* createAstIfElse(Span loc, AstNode* cond, AstNode* if_block, AstNode* else_block) {
    AstIfElse* node = NEW(AstIfElse);
    initAstNode((AstNode*)node, AST_IF_ELSE, loc);
    node->condition = cond; 
    node->if_block = if_block;
    node->else_block = else_block;
    SET_PARENT(cond);
    SET_PARENT(if_block);
    SET_PARENT(else_block);
    return node;
}

AstVar* createAstVar(Span loc, Symbol name) {
    AstVar* node = NEW(AstVar);
    initAstNode((AstNode*)node, AST_VAR, loc);
    node->name = name;
    node->binding = NULL;
    return node;
}

AstVarDef* createAstVarDef(Span loc, AstVar* name, AstNode* type, AstNode* val) {
    AstVarDef* node = NEW(AstVarDef);
    initAstNode((AstNode*)node, AST_VARDEF, loc);
    node->name = name;
    node->type = type;
    node->val = val;
    SET_PARENT(name);
    SET_PARENT(type);
    SET_PARENT(val);
    return node;
}

AstWhile* createAstWhile(Span loc, AstNode* cond, AstNode* block) {
    AstWhile* node = NEW(AstWhile);
    initAstNode((AstNode*)node, AST_WHILE, loc);
    node->condition = cond;
    node->block = block;
    SET_PARENT(cond);
    SET_PARENT(block);
    return node;
}

AstFn* createAstFn(Span loc, AstVar* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags) {
    AstFn* node = NEW(AstFn);
    initAstNode((AstNode*)node, AST_FN, loc);
    node->name = name;
    node->arguments = arguments;
    node->ret_type = ret_type;
    node->body = body;
    node->flags = flags;
    initSymbolTable(&node->vars, NULL);
    SET_PARENT(name);
    SET_PARENT(arguments);
    SET_PARENT(ret_type);
    SET_PARENT(body);
    return node;
}

AstCall* createAstCall(Span loc, AstNode* func, AstList* arguments) {
    AstCall* node = NEW(AstCall);
    initAstNode((AstNode*)node, AST_CALL, loc);
    node->function = func;
    node->arguments = arguments;
    SET_PARENT(func);
    SET_PARENT(arguments);
    return node;
}

AstInt* createAstInt(Span loc, AstNodeKind kind, AstIntType num) {
    AstInt* node = NEW(AstInt);
    initAstNode((AstNode*)node, kind, loc);
    node->number = num;
    return node;
}

AstReal* createAstReal(Span loc, AstRealType num) {
    AstReal* node = NEW(AstReal);
    initAstNode((AstNode*)node, AST_REAL, loc);
    node->number = num;
    return node;
}

AstStr* createAstStr(Span loc, String string) {
    AstStr* node = NEW(AstStr);
    initAstNode((AstNode*)node, AST_STR, loc);
    node->string = string;
    return node;
}

AstBool* createAstBool(Span loc, bool value) {
    AstBool* node = NEW(AstBool);
    initAstNode((AstNode*)node, AST_BOOL, loc);
    node->value = value;
    return node;
}

AstTypeDef* createAstTypeDef(Span loc, AstVar* name, AstNode* value) {
    AstTypeDef* node = NEW(AstTypeDef);
    initAstNode((AstNode*)node, AST_TYPEDEF, loc);
    node->name = name;
    node->value = value;
    SET_PARENT(name);
    SET_PARENT(value);
    return node;
}

AstArgDef* createAstArgDef(Span loc, AstVar* name, AstNode* type) {
    AstArgDef* node = NEW(AstArgDef);
    initAstNode((AstNode*)node, AST_ARGDEF, loc);
    node->name = name;
    node->type = type;
    SET_PARENT(name);
    SET_PARENT(type);
    return node;
}

AstRoot* createAstRoot(Span loc, AstList* nodes) {
    AstRoot* node = NEW(AstRoot);
    initAstNode((AstNode*)node, AST_ROOT, loc);
    initSymbolTable(&node->vars, NULL);
    node->nodes = nodes;
    SET_PARENT(nodes);
    return node;
}

AstBlock* createAstBlock(Span loc, AstList* nodes) {
    AstBlock* node = NEW(AstBlock);
    initAstNode((AstNode*)node, AST_BLOCK, loc);
    initSymbolTable(&node->vars, NULL);
    node->nodes = nodes;
    SET_PARENT(nodes);
    return node;
}

AstNode* createAstSimple(Span loc, AstNodeKind kind) {
    AstNode* node = NEW(AstNode);
    initAstNode((AstNode*)node, kind, loc);
    return node;
}

AstReturn* createAstReturn(Span loc, AstNode* value) {
    AstReturn* node = NEW(AstReturn);
    initAstNode((AstNode*)node, AST_RETURN, loc);
    node->value = value;
    SET_PARENT(value);
    node->function = NULL;
    return node;
}

AstFnType* createAstFnType(Span loc, AstList* arguments, AstNode* ret_type, bool vararg) {
    AstFnType* node = NEW(AstFnType);
    initAstNode((AstNode*)node, AST_FN_TYPE, loc);
    node->arguments = arguments;
    node->ret_type = ret_type;
    node->vararg = vararg;
    SET_PARENT(arguments);
    SET_PARENT(ret_type);
    return node;
}

void freeAstNode(AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ADD_ASSIGN:
            case AST_SUB_ASSIGN:
            case AST_MUL_ASSIGN:
            case AST_DIV_ASSIGN:
            case AST_MOD_ASSIGN:
            case AST_SHL_ASSIGN:
            case AST_SHR_ASSIGN:
            case AST_BAND_ASSIGN:
            case AST_BOR_ASSIGN:
            case AST_BXOR_ASSIGN:
            case AST_ASSIGN:
            case AST_INDEX:
            case AST_AS:
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
            case AST_NOT:
            case AST_SIZEOF:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                freeAstNode(n->op);
                break;
            }
            case AST_STRUCT_TYPE:
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    freeAstNode(n->nodes[i]);
                }
                FREE(n->nodes);
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                deinitSymbolTable(&n->vars);
                freeAstNode((AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                deinitSymbolTable(&n->vars);
                freeAstNode((AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                freeAstNode((AstNode*)n->name);
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
            case AST_FN_TYPE: {
                AstFnType* n = (AstFnType*)node;
                freeAstNode((AstNode*)n->arguments);
                freeAstNode(n->ret_type);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                deinitSymbolTable(&n->vars);
                freeAstNode((AstNode*)n->name);
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
                freeString(n->string);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                freeAstNode((AstNode*)n->name);
                freeAstNode(n->value);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                freeAstNode((AstNode*)n->name);
                freeAstNode(n->type);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                freeAstNode(n->value);
                break;
            }
            case AST_VAR:
            case AST_ERROR:
            case AST_VOID:
            case AST_BOOL:
            case AST_INT:
            case AST_CHAR:
            case AST_REAL: break;
        }
        FREE(node);
    }
}

