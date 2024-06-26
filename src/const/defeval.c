
#include "ast/ast.h"
#include "ast/astwalk.h"
#include "const/eval.h"
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

static void evaluateConstantDefinitionTypes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_CONSTDEF:
                typeInferExpr(context, node, NULL);
                break;
            default:
                break;
        }
        AST_FOR_EACH_CHILD(node, false, false, true, {
            evaluateConstantDefinitionTypes(context, child);
        });
    }
}

static void evaluateConstantDefinitionValues(CompilerContext* context, AstNode* node) {
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
            evaluateConstantDefinitionValues(context, child);
        });
    }
}

void runConstantValueEvaluation(CompilerContext* context) {
    FOR_ALL_MODULES({
        evaluateConstantDefinitionTypes(context, file->ast);
    });
    FOR_ALL_MODULES({
        evaluateConstantDefinitionValues(context, file->ast);
    });
}

