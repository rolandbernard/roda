
#include "ast/ast.h"
#include "ast/astwalk.h"
#include "const/eval.h"
#include "types/check.h"
#include "types/infer.h"

#include "const/defeval.h"

static void evaluateGlobalInitializers(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STATICDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL && checkValidInConstExpr(context, n->val)) {
                    SymbolVariable* var = (SymbolVariable*)n->name->binding;
                    var->value = evaluateConstExpr(context, n->val);
                }
                break;
            }
            default:
                break;
        }
        AST_FOR_EACH_CHILD(node, false, false, true, {
            evaluateGlobalInitializers(context, child);
        });
    }
}

void runGlobalInitEvaluation(CompilerContext* context) {
    FOR_ALL_MODULES_IF_OK({
        evaluateGlobalInitializers(context, file->ast);
    });
}

static void evaluateConstantValueDefinitions(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_CONSTDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL) {
                    evaluateConstantDefinition(context, n);
                }
                break;
            }
            default:
                break;
        }
        AST_FOR_EACH_CHILD(node, false, false, true, {
            evaluateConstantValueDefinitions(context, child);
        });
    }
}

void runConstantValueEvaluation(CompilerContext* context) {
    FOR_ALL_MODULES({
        evaluateConstantValueDefinitions(context, file->ast);
    });
}

