#ifndef _RODA_AST_WALK_H_
#define _RODA_AST_WALK_H_

#include "ast/ast.h"

#define AST_CHILD_HELPER(NODE, TYPE, ACTION) {      \
    bool is_type = TYPE;                            \
    AstNode* child = (AstNode*)(NODE);              \
    ACTION                                          \
}

#define AST_FOR_ALL_CHILD(NODE, TYPE, ACTION) {                         \
    switch (node->kind) {                                               \
        case AST_ADD_ASSIGN:                                            \
        case AST_SUB_ASSIGN:                                            \
        case AST_MUL_ASSIGN:                                            \
        case AST_DIV_ASSIGN:                                            \
        case AST_MOD_ASSIGN:                                            \
        case AST_SHL_ASSIGN:                                            \
        case AST_SHR_ASSIGN:                                            \
        case AST_BAND_ASSIGN:                                           \
        case AST_BOR_ASSIGN:                                            \
        case AST_BXOR_ASSIGN:                                           \
        case AST_ASSIGN: { /* right to left */                          \
            AstBinary* n = (AstBinary*)(NODE);                          \
            AST_CHILD_HELPER(n->right, TYPE, ACTION);                   \
            AST_CHILD_HELPER(n->left, TYPE, ACTION);                    \
            break;                                                      \
        }                                                               \
        case AST_AS: {                                                  \
            AstBinary* n = (AstBinary*)(NODE);                          \
            AST_CHILD_HELPER(n->left, TYPE, ACTION);                    \
            AST_CHILD_HELPER(n->right, true, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_INDEX:                                                 \
        case AST_SUB:                                                   \
        case AST_MUL:                                                   \
        case AST_DIV:                                                   \
        case AST_MOD:                                                   \
        case AST_OR:                                                    \
        case AST_AND:                                                   \
        case AST_SHL:                                                   \
        case AST_SHR:                                                   \
        case AST_BAND:                                                  \
        case AST_BOR:                                                   \
        case AST_BXOR:                                                  \
        case AST_EQ:                                                    \
        case AST_NE:                                                    \
        case AST_LE:                                                    \
        case AST_GE:                                                    \
        case AST_LT:                                                    \
        case AST_GT:                                                    \
        case AST_ADD: { /* left to right */                             \
            AstBinary* n = (AstBinary*)node;                            \
            AST_CHILD_HELPER(n->left, TYPE, ACTION);                    \
            AST_CHILD_HELPER(n->right, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_ARRAY: {                                               \
            AstBinary* n = (AstBinary*)node;                            \
            AST_CHILD_HELPER(n->right, TYPE, ACTION);                   \
            AST_CHILD_HELPER(n->left, false, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_POS:                                                   \
        case AST_NEG:                                                   \
        case AST_ADDR:                                                  \
        case AST_NOT:                                                   \
        case AST_DEREF: {                                               \
            AstUnary* n = (AstUnary*)node;                              \
            AST_CHILD_HELPER(n->op, TYPE, ACTION);                      \
            break;                                                      \
        }                                                               \
        case AST_SIZEOF: {                                              \
            AstUnary* n = (AstUnary*)node;                              \
            AST_CHILD_HELPER(n->op, true, ACTION);                      \
            break;                                                      \
        }                                                               \
        case AST_RETURN: {                                              \
            AstReturn* n = (AstReturn*)node;                            \
            AST_CHILD_HELPER(n->value, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_STRUCT_TYPE: {                                         \
            AstList* n = (AstList*)node;                                \
            for (size_t i = 0; i < n->count; i++) {                     \
                AstStructField* field = (AstStructField*)n->nodes[i];   \
                AST_CHILD_HELPER(field->type, true, ACTION);            \
            }                                                           \
            break;                                                      \
        }                                                               \
        case AST_STRUCT_LIT: {                                          \
            AstList* n = (AstList*)node;                                \
            for (size_t i = 0; i < n->count; i++) {                     \
                AstStructField* field = (AstStructField*)n->nodes[i];   \
                AST_CHILD_HELPER(field->field_value, TYPE, ACTION);     \
            }                                                           \
            break;                                                      \
        }                                                               \
        case AST_TUPLE_TYPE:                                            \
        case AST_TUPLE_LIT:                                             \
        case AST_ARRAY_LIT:                                             \
        case AST_LIST: {                                                \
            AstList* n = (AstList*)node;                                \
            for (size_t i = 0; i < n->count; i++) {                     \
                AST_CHILD_HELPER(n->nodes[i], TYPE, ACTION);            \
            }                                                           \
            break;                                                      \
        }                                                               \
        case AST_ROOT: {                                                \
            AstRoot* n = (AstRoot*)node;                                \
            AST_CHILD_HELPER(n->nodes, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_BLOCK_EXPR:                                            \
        case AST_BLOCK: {                                               \
            AstBlock* n = (AstBlock*)node;                              \
            AST_CHILD_HELPER(n->nodes, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_STATICDEF:                                             \
        case AST_CONSTDEF: {                                            \
            AstVarDef* n = (AstVarDef*)node;                            \
            AST_CHILD_HELPER(n->type, true, ACTION);                    \
            AST_CHILD_HELPER(n->val, TYPE, ACTION);                     \
            break;                                                      \
        }                                                               \
        case AST_VARDEF: {                                              \
            AstVarDef* n = (AstVarDef*)node;                            \
            AST_CHILD_HELPER(n->type, true, ACTION);                    \
            AST_CHILD_HELPER(n->val, TYPE, ACTION);                     \
            break;                                                      \
        }                                                               \
        case AST_IF_ELSE_EXPR:                                          \
        case AST_IF_ELSE: {                                             \
            AstIfElse* n = (AstIfElse*)node;                            \
            AST_CHILD_HELPER(n->condition, TYPE, ACTION);               \
            AST_CHILD_HELPER(n->if_block, TYPE, ACTION);                \
            AST_CHILD_HELPER(n->else_block, TYPE, ACTION);              \
            break;                                                      \
        }                                                               \
        case AST_WHILE: {                                               \
            AstWhile* n = (AstWhile*)node;                              \
            AST_CHILD_HELPER(n->condition, TYPE, ACTION);               \
            AST_CHILD_HELPER(n->block, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_FN_TYPE: {                                             \
            AstFnType* n = (AstFnType*)node;                            \
            AST_CHILD_HELPER(n->arguments, TYPE, ACTION);               \
            AST_CHILD_HELPER(n->ret_type, true, ACTION);                \
            break;                                                      \
        }                                                               \
        case AST_FN: {                                                  \
            AstFn* n = (AstFn*)node;                                    \
            AST_CHILD_HELPER(n->ret_type, true, ACTION);                \
            AST_CHILD_HELPER(n->arguments, TYPE, ACTION);               \
            AST_CHILD_HELPER(n->body, TYPE, ACTION);                    \
            break;                                                      \
        }                                                               \
        case AST_CALL: {                                                \
            AstCall* n = (AstCall*)node;                                \
            AST_CHILD_HELPER(n->function, TYPE, ACTION);                \
            AST_CHILD_HELPER(n->arguments, TYPE, ACTION);               \
            break;                                                      \
        }                                                               \
        case AST_TYPEDEF: {                                             \
            AstTypeDef* n = (AstTypeDef*)node;                          \
            AST_CHILD_HELPER(n->value, true, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_ARGDEF: {                                              \
            AstArgDef* n = (AstArgDef*)node;                            \
            AST_CHILD_HELPER(n->type, true, ACTION);                    \
            break;                                                      \
        }                                                               \
        case AST_STRUCT_INDEX: {                                        \
            AstStructIndex* n = (AstStructIndex*)node;                  \
            AST_CHILD_HELPER(n->strct, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_TUPLE_INDEX: {                                         \
            AstTupleIndex* n = (AstTupleIndex*)node;                    \
            AST_CHILD_HELPER(n->tuple, TYPE, ACTION);                   \
            break;                                                      \
        }                                                               \
        case AST_VAR:                                                   \
        case AST_ERROR:                                                 \
        case AST_BREAK:                                                 \
        case AST_CONTINUE:                                              \
        case AST_VOID:                                                  \
        case AST_STR:                                                   \
        case AST_INT:                                                   \
        case AST_CHAR:                                                  \
        case AST_BOOL:                                                  \
        case AST_REAL: break;                                           \
    }                                                                   \
}

#define AST_FOR_EACH_CHILD(NODE, INTYPE, TYPE, EXPR, ACTION)    \
    AST_FOR_ALL_CHILD(NODE, INTYPE, {                           \
        if (((TYPE) && is_type) || ((EXPR) && !is_type)) {      \
            ACTION                                              \
        }                                                       \
    })

#endif
