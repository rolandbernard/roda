
#include "ast/ast.h"
#include "errors/fatalerror.h"

#include "compiler/controlflow.h"

static bool controlFlowEndsWithReturn(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ARRAY:
            case AST_FN_TYPE:
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
                UNREACHABLE("should not evaluate");
            case AST_ERROR:
                return true;
            case AST_VAR:
            case AST_VOID:
            case AST_STR:
            case AST_CHAR:
            case AST_INT:
            case AST_REAL:
            case AST_BOOL:
                return false;
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
            case AST_ASSIGN: { // Executed right to left
                AstBinary* n = (AstBinary*)node;
                return controlFlowEndsWithReturn(context, n->right)
                    || controlFlowEndsWithReturn(context, n->left);
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                return controlFlowEndsWithReturn(context, n->left);
            }
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
            case AST_ADD: { // Executed left to right
                AstBinary* n = (AstBinary*)node;
                return controlFlowEndsWithReturn(context, n->left)
                    || controlFlowEndsWithReturn(context, n->right);
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                return controlFlowEndsWithReturn(context, n->op);
            }
            case AST_SIZEOF:
                return false;
            case AST_RETURN:
                return true;
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    if (controlFlowEndsWithReturn(context, field->field_value)) {
                        // TODO: warning about dead code?
                        return true;
                    }
                }
                return false;
            }
            case AST_TUPLE_LIT:
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    if (controlFlowEndsWithReturn(context, n->nodes[i])) {
                        // TODO: warning about dead code?
                        return true;
                    }
                }
                return false;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                return controlFlowEndsWithReturn(context, (AstNode*)n->nodes);
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                return controlFlowEndsWithReturn(context, (AstNode*)n->nodes);
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                return controlFlowEndsWithReturn(context, n->val);
            }
            case AST_IF_ELSE_EXPR:
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                return controlFlowEndsWithReturn(context, n->condition)
                    || (
                        controlFlowEndsWithReturn(context, n->if_block)
                        && controlFlowEndsWithReturn(context, n->else_block)
                    );
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                return controlFlowEndsWithReturn(context, n->condition);
            }
            case AST_FN:
            case AST_TYPEDEF:
            case AST_ARGDEF:
                return false;
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                return controlFlowEndsWithReturn(context, n->function)
                    || controlFlowEndsWithReturn(context, (AstNode*)n->arguments);
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                return controlFlowEndsWithReturn(context, n->strct);
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                return controlFlowEndsWithReturn(context, n->tuple);
            }
        }
        UNREACHABLE();
    } else {
        return false;
    }
}

static void checkControlFlowConstraints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    checkControlFlowConstraints(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                checkControlFlowConstraints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (n->ret_type != NULL && n->body != NULL && !controlFlowEndsWithReturn(context, n->body)) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_MISSING_RETURN,
                        copyFromCString("function body reaches end of control flow without expected return"), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("function expected to return a value"), node->location)
                    ));
                }
                break;
            }
            default:
                // We only care about functions, which can only be at top level
                break;
        }
    }
}

void runControlFlowChecking(CompilerContext* context) {
    FOR_ALL_MODULES({ checkControlFlowConstraints(context, file->ast); });
    // TODO: Check no use of uninitialized?
}

