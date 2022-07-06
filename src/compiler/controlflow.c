
#include "ast/ast.h"
#include "ast/astwalk.h"
#include "errors/fatalerror.h"

#include "compiler/controlflow.h"

static bool controlFlowEndsWithReturn(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ERROR:
                return true;
            case AST_RETURN:
                return true;
            case AST_STATICDEF:
            case AST_CONSTDEF:
                return false;
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
            default:
                AST_FOR_EACH_CHILD(node, false, false, true, {
                    if (controlFlowEndsWithReturn(context, child)) {
                        return true;
                    }
                });
                return false;
        }
        UNREACHABLE();
    } else {
        return false;
    }
}

static void checkControlFlowConstraints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
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
                break;
        }
        AST_FOR_EACH_CHILD(node, false, false, true, {
            checkControlFlowConstraints(context, child);
        });
    }
}

void runControlFlowChecking(CompilerContext* context) {
    FOR_ALL_MODULES({ checkControlFlowConstraints(context, file->ast); });
}

