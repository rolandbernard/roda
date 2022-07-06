
#include "ast/ast.h"
#include "consteval/eval.h"
#include "types/check.h"
#include "types/infer.h"

#include "consteval/defeval.h"

static void evaluateConstantValueDefinitions(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    evaluateConstantValueDefinitions(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                evaluateConstantValueDefinitions(context, (AstNode*)n->nodes);
                break;
            }
            case AST_STATICDEF:
            case AST_CONSTDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL) {
                    evaluateConstantDefinition(context, n);
                }
                break;
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

void runConstantValueEvaluation(CompilerContext* context) {
    FOR_ALL_MODULES({
        evaluateConstantValueDefinitions(context, file->ast);
    });
}

