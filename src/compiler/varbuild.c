
#include "ast/ast.h"
#include "errors/fatalerror.h"
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

static void buildLocalSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope, bool type) {
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
                buildLocalSymbolTables(context, n->right, scope, type);
                buildLocalSymbolTables(context, n->left, scope, type);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                buildLocalSymbolTables(context, n->left, scope, type);
                buildLocalSymbolTables(context, n->right, scope, true);
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
                buildLocalSymbolTables(context, n->left, scope, type);
                buildLocalSymbolTables(context, n->right, scope, type);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                buildLocalSymbolTables(context, n->right, scope, true);
                buildLocalSymbolTables(context, n->left, scope, false);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                buildLocalSymbolTables(context, n->op, scope, type);
                break;
            }
            case AST_SIZEOF: {
                AstUnary* n = (AstUnary*)node;
                buildLocalSymbolTables(context, n->op, scope, true);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                buildLocalSymbolTables(context, n->value, scope, type);
                break;
            }
            case AST_STRUCT_TYPE: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    buildLocalSymbolTables(context, field->type, scope, true);
                }
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    buildLocalSymbolTables(context, field->field_value, scope, type);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildLocalSymbolTables(context, n->nodes[i], scope, type);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                buildLocalSymbolTables(context, (AstNode*)n->nodes, &n->vars, type);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = scope;
                buildLocalSymbolTables(context, (AstNode*)n->nodes, &n->vars, type);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                buildLocalSymbolTables(context, n->type, scope, true);
                buildLocalSymbolTables(context, n->val, scope, type);
                n->name->binding = (SymbolEntry*)createVariableSymbol(n->name->name, n->name);
                addSymbolToTable(scope, n->name->binding);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                buildLocalSymbolTables(context, n->condition, scope, type);
                buildLocalSymbolTables(context, n->if_block, scope, type);
                buildLocalSymbolTables(context, n->else_block, scope, type);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                buildLocalSymbolTables(context, n->condition, scope, type);
                buildLocalSymbolTables(context, n->block, scope, type);
                break;
            }
            case AST_FN_TYPE: {
                AstFnType* n = (AstFnType*)node;
                buildLocalSymbolTables(context, (AstNode*)n->arguments, scope, type);
                buildLocalSymbolTables(context, n->ret_type, scope, type);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                buildLocalSymbolTables(context, n->ret_type, scope, true);
                n->vars.parent = scope;
                buildLocalSymbolTables(context, (AstNode*)n->arguments, &n->vars, type);
                buildLocalSymbolTables(context, n->body, &n->vars, type);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                buildLocalSymbolTables(context, n->function, scope, type);
                buildLocalSymbolTables(context, (AstNode*)n->arguments, scope, type);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                buildLocalSymbolTables(context, n->value, scope, true);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                buildLocalSymbolTables(context, n->type, scope, true);
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
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                buildLocalSymbolTables(context, n->strct, scope, type);
                break;
            }
            case AST_ERROR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_CHAR:
            case AST_BOOL:
            case AST_REAL: break;
        }
    }
}

static void buildRootSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* scope) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildRootSymbolTables(context, n->nodes[i], scope);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = scope;
                buildRootSymbolTables(context, (AstNode*)n->nodes, &n->vars);
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

void runSymbolResolution(CompilerContext* context) {
    FOR_ALL_MODULES({ buildRootSymbolTables(context, file->ast, &context->buildins); });
    FOR_ALL_MODULES({ buildLocalSymbolTables(context, file->ast, &context->buildins, false); });
}

static void buildControlFlowReferences(CompilerContext* context, AstNode* node, AstFn* function) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_ARRAY:
            case AST_FN_TYPE:
                UNREACHABLE("should not evaluate");
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
                buildControlFlowReferences(context, n->right, function);
                buildControlFlowReferences(context, n->left, function);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                buildControlFlowReferences(context, n->left, function);
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
                buildControlFlowReferences(context, n->left, function);
                buildControlFlowReferences(context, n->right, function);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                buildControlFlowReferences(context, n->op, function);
                break;
            }
            case AST_SIZEOF:
                break;
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                n->function = function;
                buildControlFlowReferences(context, n->value, function);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    buildControlFlowReferences(context, field->field_value, function);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildControlFlowReferences(context, n->nodes[i], function);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildControlFlowReferences(context, (AstNode*)n->nodes, function);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                buildControlFlowReferences(context, (AstNode*)n->nodes, function);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                buildControlFlowReferences(context, n->val, function);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                buildControlFlowReferences(context, n->condition, function);
                buildControlFlowReferences(context, n->if_block, function);
                buildControlFlowReferences(context, n->else_block, function);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                buildControlFlowReferences(context, n->condition, function);
                buildControlFlowReferences(context, n->block, function);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                buildControlFlowReferences(context, (AstNode*)n->arguments, n);
                buildControlFlowReferences(context, n->body, n);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                buildControlFlowReferences(context, n->function, function);
                buildControlFlowReferences(context, (AstNode*)n->arguments, function);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                buildControlFlowReferences(context, n->strct, function);
                break;
            }
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_VAR:
            case AST_ERROR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_CHAR:
            case AST_BOOL:
            case AST_REAL: break;
        }
    }
}

void runControlFlowReferenceResolution(CompilerContext* context) {
    FOR_ALL_MODULES({ buildControlFlowReferences(context, file->ast, NULL); });
}

