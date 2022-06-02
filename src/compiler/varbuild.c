
#include "errors/fatalerror.h"
#include "text/format.h"
#include "text/string.h"

#include "compiler/varbuild.h"

static void variableExistsError(CompilerContext* context, AstVar* name, SymbolEntry* existing) {
    const char* kind_name = existing->kind == SYMBOL_VARIABLE ? "variable" : "type";
    String message = createFormattedString("the %s `%s` is already defined", kind_name, name->name);
    MessageFragment* error = createMessageFragment(MESSAGE_ERROR, createFormattedString("already defined %s", kind_name), name->location);
    if (existing->def != NULL) {
        addMessageToContext(&context->msgs, createMessage(ERROR_ALREADY_DEFINED, message, 1, error));
    } else {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ALREADY_DEFINED, message, 2, error,
                createMessageFragment(MESSAGE_NOTE, copyFromCString("note: previously defined here"), existing->def->location)
            )
        );
    }
}

static bool checkNotExisting(CompilerContext* context, SymbolTable* scope, AstVar* name, SymbolEntryKind kind) {
    SymbolEntry* existing = findImmediateEntryInTable(scope, name->name, kind);
    if (existing != NULL) {
        variableExistsError(context, name, existing);
        return false;
    } else {
        return true;
    }
}

static void recursivelyBuildSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope, bool type) {
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
                recursivelyBuildSymbolTables(context, n->right, scope, type);
                recursivelyBuildSymbolTables(context, n->left, scope, type);
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
                recursivelyBuildSymbolTables(context, n->left, scope, type);
                recursivelyBuildSymbolTables(context, n->right, scope, type);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildSymbolTables(context, n->right, scope, true);
                recursivelyBuildSymbolTables(context, n->left, scope, false);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                recursivelyBuildSymbolTables(context, n->op, scope, type);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildSymbolTables(context, n->nodes[i], scope, type);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                recursivelyBuildSymbolTables(context, (AstNode*)n->nodes, &n->vars, type);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                recursivelyBuildSymbolTables(context, (AstNode*)n->nodes, &n->vars, type);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                recursivelyBuildSymbolTables(context, n->type, scope, true);
                recursivelyBuildSymbolTables(context, n->val, scope, type);
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    addSymbolToTable(scope, (SymbolEntry*)createVariableSymbol(n->name->name, n->name));
                }
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                recursivelyBuildSymbolTables(context, n->condition, scope, type);
                recursivelyBuildSymbolTables(context, n->if_block, scope, type);
                recursivelyBuildSymbolTables(context, n->else_block, scope, type);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                recursivelyBuildSymbolTables(context, n->condition, scope, type);
                recursivelyBuildSymbolTables(context, n->block, scope, type);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    addSymbolToTable(scope, (SymbolEntry*)createVariableSymbol(n->name->name, n->name));
                }
                recursivelyBuildSymbolTables(context, n->ret_type, scope, true);
                n->vars.parent = scope;
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, &n->vars, type);
                recursivelyBuildSymbolTables(context, n->body, &n->vars, type);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                recursivelyBuildSymbolTables(context, n->function, scope, type);
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, scope, type);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                recursivelyBuildSymbolTables(context, n->value, scope, true);
                if (checkNotExisting(context, scope, n->name, SYMBOL_TYPE)) {
                    addSymbolToTable(scope, (SymbolEntry*)createTypeSymbol(n->name->name, n->name));
                }
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                recursivelyBuildSymbolTables(context, n->type, scope, true);
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    addSymbolToTable(scope, (SymbolEntry*)createVariableSymbol(n->name->name, n->name));
                }
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                n->binding = findEntryInTable(scope, n->name, type ? SYMBOL_TYPE : SYMBOL_VARIABLE);
                if (n->binding == NULL) {
                    const char* kind_name = type ? "type" : "variable";
                    String err_msg = createFormattedString("use of undefined %s `%s`", kind_name, n->name);
                    MessageFragment* err_frag = createMessageFragment(
                        MESSAGE_ERROR, createFormattedString("undefined %s", kind_name), n->location
                    );
                    if (findEntryInTable(scope, n->name, type ? SYMBOL_VARIABLE : SYMBOL_TYPE) != NULL) {
                        MessageFragment* note_frag = createMessageFragment(
                            MESSAGE_NOTE, createFormattedString("note: a %s exists with the same name", type ? "variable" : "type"), invalidSpan()
                        );
                        addMessageToContext(&context->msgs, createMessage(ERROR_UNDEFINED, err_msg, 2, err_frag, note_frag));
                    } else {
                        addMessageToContext(&context->msgs, createMessage(ERROR_UNDEFINED, err_msg, 1, err_frag));
                    }
                }
                break;
            }
            case AST_ERROR:
            case AST_STR:
            case AST_INT:
            case AST_REAL: break;
            default:
                UNREACHABLE(", unhandled ast kind");
        }
    }
}

void buildSymbolTables(CompilerContext* context, AstNode* root) {
    ASSERT(root->kind == AST_ROOT);
    recursivelyBuildSymbolTables(context, root, &context->buildins, false);
}
