
#include "ast/ast.h"
#include "ast/astprinter.h"
#include "ast/astwalk.h"
#include "text/format.h"
#include "text/string.h"

#include "compiler/varbuild.h"

static void raiseVariableExistsError(CompilerContext* context, AstVar* name, SymbolEntry* existing) {
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
        raiseVariableExistsError(context, name, existing);
        return false;
    } else {
        return true;
    }
}

static void buildLocalSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope, bool type, SymbolVariable* func) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                scope = &n->vars;
                buildLocalSymbolTables(context, (AstNode*)n->nodes, scope, type, func);
                break;
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                scope = &n->vars;
                buildLocalSymbolTables(context, (AstNode*)n->nodes, scope, type, func);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                n->vars.parent = scope;
                buildLocalSymbolTables(context, n->ret_type, scope, true, func);
                scope = &n->vars;
                buildLocalSymbolTables(context, (AstNode*)n->arguments, scope, type, (SymbolVariable*)n->name->binding);
                buildLocalSymbolTables(context, n->body, scope, type, (SymbolVariable*)n->name->binding);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                buildLocalSymbolTables(context, n->type, scope, true, func);
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name, false, func);
                    addSymbolToTable(scope, n->name->binding);
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                buildLocalSymbolTables(context, n->type, scope, true, func);
                buildLocalSymbolTables(context, n->val, scope, type, func);
                n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name, false, func);
                addSymbolToTable(scope, n->name->binding);
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
                } else if (!type) {
                    SymbolVariable* var_func = ((SymbolVariable*)n->binding)->function;
                    if (var_func != NULL && var_func != func) {
                        String message = createFormattedString(
                            "can not capture variable `%s` from outer function", n->name
                        );
                        MessageFragment* error = createMessageFragment(
                            MESSAGE_ERROR, copyFromCString("can not capture variable"), n->location
                        );
                        if (n->binding->def != NULL) {
                            addMessageToContext(
                                &context->msgs,
                                createMessage(
                                    ERROR_ALREADY_DEFINED, message, 2, error,
                                    createMessageFragment(
                                        MESSAGE_NOTE,
                                        copyFromCString("note: variable defined here"),
                                        n->binding->def->location
                                    )
                                )
                            );
                        } else {
                            addMessageToContext(
                                &context->msgs,
                                createMessage(ERROR_UNDEFINED, message, 1, error)
                            );
                        }
                    } else {
                        addSymbolReference(n->binding, n);
                    }
                } else {
                    addSymbolReference(n->binding, n);
                }
                break;
            }
            default:
                AST_FOR_EACH_CHILD(node, type, true, true, {
                    buildLocalSymbolTables(context, child, scope, is_type, func);
                });
                break;
        }
    }
}

static void buildRootSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                scope = &n->vars;
                buildRootSymbolTables(context, (AstNode*)n->nodes, scope);
                break;
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                scope = &n->vars;
                buildRootSymbolTables(context, (AstNode*)n->nodes, scope);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name, false, NULL);
                    addSymbolToTable(scope, n->name->binding);
                }
                scope = &n->vars;
                buildRootSymbolTables(context, (AstNode*)n->arguments, scope);
                buildRootSymbolTables(context, n->body, scope);
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
            case AST_STATICDEF:
            case AST_CONSTDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (checkNotExisting(context, scope, n->name, SYMBOL_VARIABLE)) {
                    n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name, node->kind == AST_CONSTDEF, NULL);
                    addSymbolToTable(scope, n->name->binding);
                }
                buildRootSymbolTables(context, n->val, scope);
                break;
            }
            default:
                AST_FOR_EACH_CHILD(node, false, false, true, {
                    buildRootSymbolTables(context, child, scope);
                });
                break;
        }
    }
}

void runSymbolResolution(CompilerContext* context) {
    FOR_ALL_MODULES({ buildRootSymbolTables(context, file->ast, &context->buildins); });
    FOR_ALL_MODULES({ buildLocalSymbolTables(context, file->ast, &context->buildins, false, NULL); });
}

typedef struct {
    AstFn* function;
    AstNode* break_target;
} ControlFlowRefBuildContext;

static void raiseControlFlowTargetMissingError(CompilerContext* context, AstNode* node) {
    String message = createFormattedString("no target for %s expression", getAstPrintName(node->kind));
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("not inside %s target", getAstPrintName(node->kind)), node->location
    );
    addMessageToContext(&context->msgs, createMessage(ERROR_NO_SUCH_TARGET, message, 1, error));
}

static void buildControlFlowReferences(CompilerContext* context, ControlFlowRefBuildContext* data, AstNode* node, bool type) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ARRAY: {
                AstBinary* n = (AstBinary*)node;
                buildControlFlowReferences(context, data, n->right, type);
                AstFn* prev_func = data->function;
                AstNode* prev_break = data->break_target;
                data->function = NULL;
                data->break_target = NULL;
                buildControlFlowReferences(context, data, n->left, false);
                data->function = prev_func;
                data->break_target = prev_break;
                break;
            }
            case AST_CONTINUE:
            case AST_BREAK: {
                if (data->break_target == NULL) {
                    raiseControlFlowTargetMissingError(context, node);
                } else {
                    AstBreak* n = (AstBreak*)node;
                    n->break_target = data->break_target;
                }
                break;
            }
            case AST_RETURN: {
                if (data->function == NULL) {
                    raiseControlFlowTargetMissingError(context, node);
                } else {
                    AstReturn* n = (AstReturn*)node;
                    n->function = data->function;
                    buildControlFlowReferences(context, data, n->value, type);
                }
                break;
            }
            case AST_STATICDEF:
            case AST_CONSTDEF: {
                AstVarDef* n = (AstVarDef*)node;
                buildControlFlowReferences(context, data, n->type, true);
                AstFn* prev_func = data->function;
                AstNode* prev_break = data->break_target;
                data->function = NULL;
                data->break_target = NULL;
                buildControlFlowReferences(context, data, n->val, type);
                data->function = prev_func;
                data->break_target = prev_break;
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                buildControlFlowReferences(context, data, n->condition, type);
                AstNode* prev = data->break_target;
                data->break_target = (AstNode*)n;
                buildControlFlowReferences(context, data, n->block, type);
                data->break_target = prev;
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                buildControlFlowReferences(context, data, n->ret_type, true);
                AstFn* prev = data->function;
                data->function = n;
                buildControlFlowReferences(context, data, (AstNode*)n->arguments, type);
                buildControlFlowReferences(context, data, n->body, type);
                data->function = prev;
                break;
            }
            default:
                AST_FOR_EACH_CHILD(node, type, true, true, {
                    buildControlFlowReferences(context, data, child, is_type);
                });
                break;
        }
    }
}

void runControlFlowReferenceResolution(CompilerContext* context) {
    ControlFlowRefBuildContext data = {
        .function = NULL,
        .break_target = NULL,
    };
    FOR_ALL_MODULES({
        buildControlFlowReferences(context, &data, file->ast, false);
    });
}

