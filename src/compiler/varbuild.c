
#include "ast/ast.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "text/string.h"

#include "compiler/varbuild.h"

static void variableExistsError(CompilerContext* context, AstVar* name, SymbolEntry* existing) {
    const char* kind_name = existing->kind == SYMBOL_VARIABLE ? "variable" : "type";
    String message = createFormattedString("the %s `%s` is already defined", kind_name, name->name);
    MessageFragment* error = createMessageFragment(MESSAGE_ERROR, createFormattedString("already defined %s", kind_name), name->location);
    if (existing->def != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ALREADY_DEFINED, message, 2, error,
                createMessageFragment(MESSAGE_NOTE, copyFromCString("note: previously defined here"), existing->def->location)
            )
        );
    } else {
        addMessageToContext(&context->msgs, createMessage(ERROR_ALREADY_DEFINED, message, 1, error));
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

static void recursivelyBuildLocalSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope, bool type, bool root) {
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
                recursivelyBuildLocalSymbolTables(context, n->right, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, n->left, scope, type, root);
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
                recursivelyBuildLocalSymbolTables(context, n->left, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, n->right, scope, type, root);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildLocalSymbolTables(context, n->right, scope, true, root);
                recursivelyBuildLocalSymbolTables(context, n->left, scope, false, root);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                recursivelyBuildLocalSymbolTables(context, n->op, scope, type, root);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                recursivelyBuildLocalSymbolTables(context, n->value, scope, type, root);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildLocalSymbolTables(context, n->nodes[i], scope, type, root);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                recursivelyBuildLocalSymbolTables(context, (AstNode*)n->nodes, &n->vars, type, root);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                recursivelyBuildLocalSymbolTables(context, (AstNode*)n->nodes, &n->vars, type, root);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                recursivelyBuildLocalSymbolTables(context, n->type, scope, true, root);
                recursivelyBuildLocalSymbolTables(context, n->val, scope, type, root);
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name);
                    addSymbolToTable(scope, n->name->binding);
                }
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                recursivelyBuildLocalSymbolTables(context, n->condition, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, n->if_block, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, n->else_block, scope, type, root);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                recursivelyBuildLocalSymbolTables(context, n->condition, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, n->block, scope, type, root);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                recursivelyBuildLocalSymbolTables(context, n->ret_type, scope, true, false);
                n->vars.parent = scope;
                recursivelyBuildLocalSymbolTables(context, (AstNode*)n->arguments, &n->vars, type, false);
                recursivelyBuildLocalSymbolTables(context, n->body, &n->vars, type, false);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                recursivelyBuildLocalSymbolTables(context, n->function, scope, type, root);
                recursivelyBuildLocalSymbolTables(context, (AstNode*)n->arguments, scope, type, root);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                recursivelyBuildLocalSymbolTables(context, n->value, scope, true, false);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                recursivelyBuildLocalSymbolTables(context, n->type, scope, true, root);
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name);
                    addSymbolToTable(scope, n->name->binding);
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
                } else {
                    addSymbolReference(n->binding, n);
                }
                break;
            }
            case AST_ERROR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_REAL: break;
        }
    }
}

static void buildLocalSymbolTables(CompilerContext* context, AstNode* root) {
    ASSERT(root->kind == AST_ROOT);
    recursivelyBuildLocalSymbolTables(context, root, &context->buildins, false, true);
}

static void recursivelyBuildRootSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildRootSymbolTables(context, n->nodes[i], scope);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                recursivelyBuildRootSymbolTables(context, (AstNode*)n->nodes, &n->vars);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name);
                    addSymbolToTable(scope, n->name->binding);
                }
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                if (checkNotExisting(context, scope, n->name, SYMBOL_TYPE)) {
                    n->name->binding = (SymbolEntry*)createTypeSymbol(n->name->name, n->name);
                    addSymbolToTable(scope, n->name->binding);
                }
                break;
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

static void buildRootSymbolTable(CompilerContext* context, AstNode* root) {
    ASSERT(root->kind == AST_ROOT);
    recursivelyBuildRootSymbolTables(context, root, &context->buildins);
}

void runSymbolResolution(CompilerContext* context) {
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        if (file->ast != NULL) {
            buildRootSymbolTable(context, file->ast);
        }
    }
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        if (file->ast != NULL) {
            buildLocalSymbolTables(context, file->ast);
        }
    }
}

static void recursivelyBuildControlFlowReferences(CompilerContext* context, AstNode* node, AstFn* function) {
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
                recursivelyBuildControlFlowReferences(context, n->right, function);
                recursivelyBuildControlFlowReferences(context, n->left, function);
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
                recursivelyBuildControlFlowReferences(context, n->left, function);
                recursivelyBuildControlFlowReferences(context, n->right, function);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildControlFlowReferences(context, n->right, function);
                recursivelyBuildControlFlowReferences(context, n->left, function);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                recursivelyBuildControlFlowReferences(context, n->op, function);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                n->function = function;
                recursivelyBuildControlFlowReferences(context, n->value, function);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildControlFlowReferences(context, n->nodes[i], function);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                recursivelyBuildControlFlowReferences(context, (AstNode*)n->nodes, function);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                recursivelyBuildControlFlowReferences(context, (AstNode*)n->nodes, function);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                recursivelyBuildControlFlowReferences(context, n->type, function);
                recursivelyBuildControlFlowReferences(context, n->val, function);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                recursivelyBuildControlFlowReferences(context, n->condition, function);
                recursivelyBuildControlFlowReferences(context, n->if_block, function);
                recursivelyBuildControlFlowReferences(context, n->else_block, function);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                recursivelyBuildControlFlowReferences(context, n->condition, function);
                recursivelyBuildControlFlowReferences(context, n->block, function);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                recursivelyBuildControlFlowReferences(context, n->ret_type, n);
                recursivelyBuildControlFlowReferences(context, (AstNode*)n->arguments, n);
                recursivelyBuildControlFlowReferences(context, n->body, n);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                recursivelyBuildControlFlowReferences(context, n->function, function);
                recursivelyBuildControlFlowReferences(context, (AstNode*)n->arguments, function);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                recursivelyBuildControlFlowReferences(context, n->value, function);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                recursivelyBuildControlFlowReferences(context, n->type, function);
                break;
            }
            case AST_VAR:
            case AST_ERROR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_REAL: break;
        }
    }
}

void runControlFlowReferenceResolution(CompilerContext* context) {
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        if (file->ast != NULL) {
            recursivelyBuildControlFlowReferences(context, file->ast, NULL);
        }
    }
}

