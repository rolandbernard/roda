
#include "ast/ast.h"
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "util/alloc.h"

#include "compiler/typecheck.h"

static void evaluateTypeHints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_NEVER:
            case AST_ARRAY: {
                UNREACHABLE(", should not evaluate");
            }
            case AST_ERROR: {
                node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                break;
            }
            case AST_VAR:
            case AST_STR:
            case AST_INT:
            case AST_REAL:
                break;
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
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, n->right);
                evaluateTypeHints(context, n->left);
                break;
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
                evaluateTypeHints(context, n->left);
                evaluateTypeHints(context, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                evaluateTypeHints(context, n->op);
                break;
            }
            case AST_RETURN: {
                AstUnary* n = (AstUnary*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, n->op);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                for (size_t i = 0; i < n->count; i++) {
                    evaluateTypeHints(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                if (n->type != NULL) {
                    SymbolVariable* var = (SymbolVariable*)n->name->binding;
                    if (var != NULL) {
                        var->type = evaluateTypeExpr(context, n->type);
                        n->name->res_type = var->type;
                    }
                }
                evaluateTypeHints(context, n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, n->condition);
                evaluateTypeHints(context, n->if_block);
                evaluateTypeHints(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, n->condition);
                evaluateTypeHints(context, n->block);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, (AstNode*)n->arguments);
                Type* ret_type;
                if (n->ret_type != NULL) {
                    ret_type = evaluateTypeExpr(context, n->ret_type);
                } else {
                    ret_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                }
                Type** arg_types = ALLOC(Type*, n->arguments->count);
                for (size_t i = 0; i < n->arguments->count; i++) {
                    AstArgDef* def = (AstArgDef*)n->arguments->nodes[i];
                    SymbolVariable* var = (SymbolVariable*)def->name->binding;
                    if (var != NULL) {
                        arg_types[i] = var->type;
                    } else {
                        arg_types[i] = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    }
                }
                SymbolVariable* func = (SymbolVariable*)n->name->binding;
                if (func != NULL) {
                    func->type = (Type*)createFunctionType(&context->types, ret_type, n->arguments->count, arg_types);
                    n->name->res_type = func->type;
                }
                evaluateTypeHints(context, n->body);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                evaluateTypeHints(context, n->function);
                evaluateTypeHints(context, (AstNode*)n->arguments);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                SymbolType* type = (SymbolType*)n->name->binding;
                if (type != NULL) {
                    type->type = evaluateTypeExpr(context, n->value);
                    n->name->res_type = type->type;
                }
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                SymbolVariable* var  = (SymbolVariable*)n->name->binding;
                if (n->type != NULL && var != NULL) {
                    var->type = evaluateTypeExpr(context, n->type);
                    n->name->res_type = var->type;
                }
                break;
            }
        }
    }
}

static void diffuseTypeFromAstNode(CompilerContext* context, AstNode* root) {
}

static void infereTypesStrong(CompilerContext* context, AstNode* root) {
    // TODO
}

static void infereTypesWeak(CompilerContext* context, AstNode* root) {
    // TODO
}

static void checkTypes(CompilerContext* context, AstNode* root) {
    // TODO
}

#define FOR_ALL_MODULES(ACTION)                                 \
    for (size_t i = 0; i < context->files.file_count; i++) {    \
        File* file = context->files.files[i];                   \
        if (file->ast != NULL) ACTION                           \
    }

void runTypeChecking(CompilerContext* context) {
    FOR_ALL_MODULES({ evaluateTypeHints(context, file->ast); });
    FOR_ALL_MODULES({ infereTypesStrong(context, file->ast); });
    FOR_ALL_MODULES({ infereTypesWeak(context, file->ast); });
    FOR_ALL_MODULES({ checkTypes(context, file->ast); });
}

