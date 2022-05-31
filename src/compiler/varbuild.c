
#include "errors/fatalerror.h"
#include "text/format.h"
#include "text/string.h"

#include "compiler/varbuild.h"

#define VARIABLE_NAME "variable"
#define TYPE_NAME "type"

static void putInSymbolTable(CompilerContext* context, SymbolTable* scope, AstVar* name, const char* kind) {
    Variable* existing = findImmediateSymbolInTable(scope, name->name);
    if (existing == NULL) {
        Variable* var = createVariable(name->name, name->location);
        addSymbolToTable(scope, var);
    } else {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ALREADY_DEFINED,
                createFormattedString("the %s `%s` is already defined", kind, name->name),
                2,
                createMessageFragment(MESSAGE_ERROR, createFormattedString("already defined %s", kind), name->location),
                createMessageFragment(MESSAGE_NOTE, copyFromCString("note: previously defined here"), existing->def_loc)
            )
        );
    }
}

static void recursivelyBuildSymbolTables(CompilerContext* context, AstNode* node, SymbolTable* vars, SymbolTable* types, bool type) {
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
                recursivelyBuildSymbolTables(context, n->right, vars, types, type);
                recursivelyBuildSymbolTables(context, n->left, vars, types, type);
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
                recursivelyBuildSymbolTables(context, n->left, vars, types, type);
                recursivelyBuildSymbolTables(context, n->right, vars, types, type);
                break;
            }
            case AST_ARRAY: { // Part of a type
                AstBinary* n = (AstBinary*)node;
                recursivelyBuildSymbolTables(context, n->right, vars, types, true);
                recursivelyBuildSymbolTables(context, n->left, vars, types, false);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                recursivelyBuildSymbolTables(context, n->op, vars, types, type);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    recursivelyBuildSymbolTables(context, n->nodes[i], vars, types, type);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                n->vars.parent = vars;
                n->types.parent = types;
                recursivelyBuildSymbolTables(context, (AstNode*)n->nodes, &n->vars, &n->types, type);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                n->vars.parent = vars;
                recursivelyBuildSymbolTables(context, (AstNode*)n->nodes, &n->vars, types, type);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                putInSymbolTable(context, vars, n->name, VARIABLE_NAME);
                recursivelyBuildSymbolTables(context, n->type, vars, types, true);
                recursivelyBuildSymbolTables(context, n->val, vars, types, type);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                recursivelyBuildSymbolTables(context, n->condition, vars, types, type);
                recursivelyBuildSymbolTables(context, n->else_block, vars, types, type);
                recursivelyBuildSymbolTables(context, n->if_block, vars, types, type);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                recursivelyBuildSymbolTables(context, n->condition, vars, types, type);
                recursivelyBuildSymbolTables(context, n->block, vars, types, type);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                putInSymbolTable(context, vars, n->name, VARIABLE_NAME);
                recursivelyBuildSymbolTables(context, n->ret_type, vars, types, true);
                n->vars.parent = vars;
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, &n->vars, types, type);
                recursivelyBuildSymbolTables(context, n->body, &n->vars, types, type);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                recursivelyBuildSymbolTables(context, n->function, vars, types, type);
                recursivelyBuildSymbolTables(context, (AstNode*)n->arguments, vars, types, type);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                recursivelyBuildSymbolTables(context, n->value, vars, types, true);
                putInSymbolTable(context, types, n->name, TYPE_NAME);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                recursivelyBuildSymbolTables(context, n->type, vars, types, true);
                putInSymbolTable(context, vars, n->name, VARIABLE_NAME);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                if (findSymbolInTable(type ? types : vars, n->name) == NULL) {
                    String err_msg = createFormattedString("use of undefined %s `%s`", type ? TYPE_NAME : VARIABLE_NAME, n->name);
                    MessageFragment* err_frag = createMessageFragment(
                        MESSAGE_ERROR, createFormattedString("undefined %s", type ? TYPE_NAME : VARIABLE_NAME), n->location
                    );
                    if (findSymbolInTable(type ? vars : types, n->name) != NULL) {
                        MessageFragment* note_frag = createMessageFragment(
                            MESSAGE_NOTE, createFormattedString("note: a %s exists with the same name", type ? VARIABLE_NAME : TYPE_NAME), invalidSpan()
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
    recursivelyBuildSymbolTables(context, root, NULL, &context->buildins, false);
}
