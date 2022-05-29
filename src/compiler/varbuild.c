
#include "errors/fatalerror.h"
#include "text/format.h"
#include "text/string.h"

#include "compiler/varbuild.h"

static void putInSymbolTable(CompilerContext* context, SymbolTable* scope, AstVar* name) {
    Variable* existing = findImmediateSymbolInTable(scope, name->name);
    if (existing == NULL) {
        Variable* var = createVariable(name->name, name->location);
        addSymbolToTable(scope, var);
    } else {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ALREADY_DEFINED,
                createFormattedString("The symbol `%S` is already defined", getSymbolName(&context->syms, name->name)),
                2,
                createMessageFragment(MESSAGE_ERROR, copyFromCString("already defined symbol"), name->location),
                createMessageFragment(MESSAGE_NOTE, copyFromCString("note: previously defined here"), existing->def_loc)
            )
        );
    }
}

static void recursivelyBuildSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope) {
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
            case AST_ASSIGN: { // Executed right to left
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildSymbolTables(context, n->right, scope);
                recursivelyBuildSymbolTables(context, n->left, scope);
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
                recursivelyBuildSymbolTables(context, n->left, scope);
                recursivelyBuildSymbolTables(context, n->right, scope);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildSymbolTables(context, n->right, scope);
                recursivelyBuildSymbolTables(context, n->left, scope);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                recursivelyBuildSymbolTables(context, n->op, scope);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildSymbolTables(context, n->nodes[i], scope);
                }
                break;
            }
            case AST_ROOT:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                recursivelyBuildSymbolTables(context, (AstNode*)n->nodes, &n->vars);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                putInSymbolTable(context, scope, n->name);
                recursivelyBuildSymbolTables(context, n->type, scope);
                recursivelyBuildSymbolTables(context, n->val, scope);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                recursivelyBuildSymbolTables(context, n->condition, scope);
                recursivelyBuildSymbolTables(context, n->else_block, scope);
                recursivelyBuildSymbolTables(context, n->if_block, scope);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                recursivelyBuildSymbolTables(context, n->condition, scope);
                recursivelyBuildSymbolTables(context, n->block, scope);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                putInSymbolTable(context, scope, n->name);
                recursivelyBuildSymbolTables(context, n->ret_type, scope);
                n->vars.parent = scope;
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, &n->vars);
                recursivelyBuildSymbolTables(context, n->body, &n->vars);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                recursivelyBuildSymbolTables(context, n->function, scope);
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, scope);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                recursivelyBuildSymbolTables(context, n->value, scope);
                putInSymbolTable(context, scope, n->name);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                recursivelyBuildSymbolTables(context, n->type, scope);
                putInSymbolTable(context, scope, n->name);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                if (findSymbolInTable(scope, n->name) == NULL) {
                    addMessageToContext(
                        &context->msgs,
                        createMessage(
                            ERROR_ALREADY_DEFINED,
                            createFormattedString("Use of undefined symbol `%S`", getSymbolName(&context->syms, n->name)),
                            1,
                            createMessageFragment(MESSAGE_ERROR, copyFromCString("undefined symbol"), n->location)
                        )
                    );
                }
                break;
            }
            case AST_ERROR:
            case AST_STR:
            case AST_INT:
            case AST_REAL: break;
        }
    }
}

void buildSymbolTables(CompilerContext* context, AstNode* root) {
    ASSERT(root->kind == AST_ROOT);
    recursivelyBuildSymbolTables(context, root, &context->buildins);
}
