
#include "ast/ast.h"
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "files/file.h"
#include "text/format.h"
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
            case AST_ASSIGN: {
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
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                evaluateTypeHints(context, n->left);
                evaluateTypeHints(context, n->right);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                n->res_type_reasoning = node;
                n->left->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                n->left->res_type_reasoning = node;
                n->right->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                n->right->res_type_reasoning = node;
                evaluateTypeHints(context, n->left);
                evaluateTypeHints(context, n->right);
                break;
            }
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                n->res_type_reasoning = node;
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
                        var->type_reasoning = n->type;
                        n->name->res_type = var->type;
                        n->res_type_reasoning = n->type;
                    }
                }
                evaluateTypeHints(context, n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                n->condition->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                n->condition->res_type_reasoning = n->condition;
                evaluateTypeHints(context, n->condition);
                evaluateTypeHints(context, n->if_block);
                evaluateTypeHints(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                n->condition->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
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
                    func->type_reasoning = node;
                    n->name->res_type = func->type;
                    n->name->res_type_reasoning = node;
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
                    var->type_reasoning = (AstNode*)n->type;
                    n->name->res_type = var->type;
                    n->name->res_type_reasoning = (AstNode*)n->type;
                }
                break;
            }
        }
    }
}

static void diffuseTypes(CompilerContext* context, AstNode* node);

static bool diffuseTypeIntoAstNode(CompilerContext* context, AstNode* node, Type* type, AstNode* reasoning) {
    if (node->res_type == NULL) {
        node->res_type = type;
        node->res_type_reasoning = reasoning;
        return true;
    } else if (!isErrorType(type) && !isErrorType(node->res_type) && !compareStructuralTypes(node->res_type, type)) {
        // TODO: better error message (type conflict for ... note: expected because {location of type hint})
        String fst_type = buildTypeName(node->res_type);
        String snd_type = buildTypeName(type);
        String message = createFormattedString("type error, conflicting types `%S` and `%S`", fst_type, snd_type);
        MessageFragment* error = createMessageFragment(MESSAGE_ERROR,
            createFormattedString("conflicting types `%S` and `%S` for this expression", fst_type, snd_type), node->location
        );
        MessageFragment* notes[2];
        size_t note_count = 0;
        if (node->res_type_reasoning != NULL) {
            notes[note_count] = createMessageFragment(MESSAGE_NOTE,
                createFormattedString("note: expecting `%S` because of this", fst_type),
                node->res_type_reasoning->location
            );
            note_count++;
        }
        if (reasoning != NULL) {
            notes[note_count] = createMessageFragment(MESSAGE_NOTE,
                createFormattedString("note: expecting `%S` because of this", snd_type),
                reasoning->location
            );
            note_count++;
        }
        if (note_count == 0) {
            addMessageToContext(&context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error));
        } else if (note_count == 1) {
            addMessageToContext(&context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 2, error, notes[0]));
        } else {
            addMessageToContext(&context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 3, error, notes[0], notes[1]));
        }
        freeString(fst_type);
        freeString(snd_type);
        node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
        return true;
    } else {
        return false;
    }
}

static bool diffuseTypeFromIntoAstNode(CompilerContext* context, AstNode* into, AstNode* from) {
    return diffuseTypeIntoAstNode(context, into, from->res_type, from->res_type_reasoning);
}

static void diffuseTypes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_NEVER:
            case AST_ARRAY: 
                UNREACHABLE(", should not evaluate");
            case AST_ERROR:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_STR:
            case AST_INT:
            case AST_REAL:
            case AST_LIST:
            case AST_ROOT:
            case AST_BLOCK:
            case AST_IF_ELSE:
            case AST_WHILE:
            case AST_FN:
                break;
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var != NULL) {
                    if (var->type != NULL) {
                        if (diffuseTypeIntoAstNode(context, node, var->type, var->type_reasoning)) {
                            diffuseTypes(context, node->parent);
                        }
                    } else if (n->res_type != NULL) {
                        var->type = n->res_type;
                        var->type_reasoning = n->res_type_reasoning;
                        for (size_t i = 0; i < var->ref_count; i++) {
                            if (n != var->refs[i]) {
                                diffuseTypes(context, (AstNode*)var->refs[i]);
                            }
                        }
                        diffuseTypes(context, (AstNode*)var->def);
                    }
                }
                break;
            }
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
            case AST_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                if (n->right->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->left, n->right)) {
                        diffuseTypes(context, n->left);
                    }
                } else if (n->left->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->right, n->left)) {
                        diffuseTypes(context, n->right);
                    }
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL) {
                    if (n->name->res_type != NULL) {
                        if (diffuseTypeFromIntoAstNode(context, n->val, (AstNode*)n->name)) {
                            diffuseTypes(context, n->val);
                        }
                    } else if (n->val->res_type != NULL) {
                        if (diffuseTypeFromIntoAstNode(context, (AstNode*)n->name, n->val)) {
                            diffuseTypes(context, (AstNode*)n->name);
                        }
                    }
                }
                break;
            }
            case AST_ADD:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_SHL:
            case AST_SHR: {
                AstBinary* n = (AstBinary*)node;
                if (n->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->left, node)) {
                        diffuseTypes(context, n->left);
                    }
                    if (diffuseTypeFromIntoAstNode(context, n->right, node)) {
                        diffuseTypes(context, n->right);
                    }
                } else if (n->left->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, node, n->left)) {
                        diffuseTypes(context, node->parent);
                    }
                    if (diffuseTypeFromIntoAstNode(context, n->right, n->left)) {
                        diffuseTypes(context, n->right);
                    }
                } else if (n->right->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, node, n->right)) {
                        diffuseTypes(context, node->parent);
                    }
                    if (diffuseTypeFromIntoAstNode(context, n->left, n->right)) {
                        diffuseTypes(context, n->left);
                    }
                }
                break;
            }
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->right, n->left)) {
                        diffuseTypes(context, n->right);
                    }
                } else if (n->right->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->left, n->right)) {
                        diffuseTypes(context, n->left);
                    }
                }
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL) {
                    TypeArray* type = isArrayType(n->left->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = n->left->res_type_reasoning;
                        if (type_reason != NULL && type_reason->kind == AST_ARRAY) {
                            AstBinary* arr_node = (AstBinary*)type_reason;
                            type_reason = arr_node->right;
                        }
                        if (diffuseTypeIntoAstNode(context, node, type->base, type_reason)) {
                            diffuseTypes(context, node->parent);
                        }
                    } else {
                        // TODO: error message
                    }
                }
                break;
            }
            case AST_OR:
            case AST_AND:
                break;
            case AST_NOT:
            case AST_POS:
            case AST_NEG: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, n->op, node)) {
                        diffuseTypes(context, n->op);
                    }
                } else if (n->op->res_type != NULL) {
                    if (diffuseTypeFromIntoAstNode(context, node, n->op)) {
                        diffuseTypes(context, node->parent);
                    }
                }
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL) {
                    if (diffuseTypeIntoAstNode(context, node, (Type*)createPointerType(&context->types, n->op->res_type), n->op->res_type_reasoning)) {
                        diffuseTypes(context, node->parent);
                    }
                } else if (n->res_type != NULL) {
                    TypePointer* type = isPointerType(n->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = node->res_type_reasoning;
                        if (type_reason != NULL && type_reason->kind == AST_ADDR) {
                            AstUnary* addr_node = (AstUnary*)type_reason;
                            type_reason = addr_node->op;
                        }
                        if (diffuseTypeIntoAstNode(context, n->op, type->base, type_reason)) {
                            diffuseTypes(context, n->op);
                        }
                    } else {
                        // TODO: error message
                    }
                }
                break;
            }
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL) {
                    TypePointer* type = isPointerType(n->op->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = n->op->res_type_reasoning;
                        if (type_reason != NULL && type_reason->kind == AST_ADDR) {
                            AstUnary* addr_node = (AstUnary*)type_reason;
                            type_reason = addr_node->op;
                        }
                        if (diffuseTypeIntoAstNode(context, node, type->base, type_reason)) {
                            diffuseTypes(context, node->parent);
                        }
                    } else {
                        // TODO: error message
                    }
                } else if (n->res_type != NULL) {
                    if (diffuseTypeIntoAstNode(context, n->op, (Type*)createPointerType(&context->types, n->res_type), node->res_type_reasoning)) {
                        diffuseTypes(context, n->op);
                    }
                }
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                if (n->function->res_type != NULL) {
                    TypeFunction* type = isFunctionType(n->function->res_type);
                    if (type == NULL) {
                        // TODO: error message
                    } else if (type->arg_count != n->arguments->count) {
                        // TODO: error message
                    } else {
                        AstNode* type_reason = n->function->res_type_reasoning;
                        if (type_reason != NULL && type_reason->kind == AST_FN) {
                            AstFn* fn_node = (AstFn*)type_reason;
                            if (fn_node->ret_type == NULL) {
                                type_reason = (AstNode*)fn_node->name;
                            } else {
                                type_reason = fn_node->ret_type;
                            }
                        }
                        if (diffuseTypeIntoAstNode(context, node, type->ret_type, type_reason)) {
                            diffuseTypes(context, node->parent);
                        }
                        for (size_t i = 0; i < n->arguments->count; i++) {
                            type_reason = n->function->res_type_reasoning;
                            if (type_reason != NULL && type_reason->kind == AST_FN) {
                                AstFn* fn_node = (AstFn*)type_reason;
                                AstArgDef* arg_node = (AstArgDef*)fn_node->arguments->nodes[i];
                                if (arg_node->type == NULL) {
                                    type_reason = (AstNode*)arg_node;
                                } else {
                                    type_reason = arg_node->type;
                                }
                            }
                            if (diffuseTypeIntoAstNode(context, n->arguments->nodes[i], type->arguments[i], type_reason)) {
                                diffuseTypes(context, n->arguments->nodes[i]);
                            }
                        }
                    }
                } else if (n->res_type != NULL) {
                    bool all_types = true;
                    for (size_t i = 0; i < n->arguments->count; i++) {
                        if (n->arguments->nodes[i]->res_type == NULL) {
                            all_types = false;
                            break;
                        }
                    }
                    if (all_types) {
                        Type** arg_types = ALLOC(Type*, n->arguments->count);
                        for (size_t i = 0; i < n->arguments->count; i++) {
                            arg_types[i] = n->arguments->nodes[i]->res_type;
                        }
                        Type* func = (Type*)createFunctionType(&context->types, n->res_type, n->arguments->count, arg_types);
                        if (diffuseTypeIntoAstNode(context, n->function, func, node)) {
                            diffuseTypes(context, n->function);
                        }
                    }
                }
                break;
            }
            case AST_RETURN: {
                // TODO: add some reference to function
                break;
            }
        }
    }
}

static void diffuseTypesOnAllNodes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        diffuseTypes(context, node);
        switch (node->kind) {
            case AST_NEVER:
            case AST_ARRAY:
                UNREACHABLE(", should not evaluate");
            case AST_ERROR:
            case AST_VAR:
            case AST_STR:
            case AST_INT:
            case AST_REAL:
            case AST_TYPEDEF:
            case AST_ARGDEF:
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
            case AST_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                diffuseTypesOnAllNodes(context, n->right);
                diffuseTypesOnAllNodes(context, n->left);
                break;
            }
            case AST_INDEX:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                diffuseTypesOnAllNodes(context, n->left);
                diffuseTypesOnAllNodes(context, n->right);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                diffuseTypesOnAllNodes(context, n->left);
                diffuseTypesOnAllNodes(context, n->right);
                break;
            }
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                diffuseTypesOnAllNodes(context, n->left);
                diffuseTypesOnAllNodes(context, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                diffuseTypesOnAllNodes(context, n->op);
                break;
            }
            case AST_RETURN: {
                AstUnary* n = (AstUnary*)node;
                diffuseTypesOnAllNodes(context, n->op);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    diffuseTypesOnAllNodes(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                diffuseTypesOnAllNodes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                diffuseTypesOnAllNodes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                diffuseTypesOnAllNodes(context, n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                diffuseTypesOnAllNodes(context, n->condition);
                diffuseTypesOnAllNodes(context, n->if_block);
                diffuseTypesOnAllNodes(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                diffuseTypesOnAllNodes(context, n->condition);
                diffuseTypesOnAllNodes(context, n->block);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                diffuseTypesOnAllNodes(context, (AstNode*)n->arguments);
                diffuseTypesOnAllNodes(context, n->body);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                diffuseTypesOnAllNodes(context, n->function);
                diffuseTypesOnAllNodes(context, (AstNode*)n->arguments);
                break;
            }
        }
    }
}

static void assumeAmbiguousTypes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_NEVER:
            case AST_ARRAY:
                UNREACHABLE(", should not evaluate");
            case AST_ERROR:
            case AST_VAR:
            case AST_TYPEDEF:
            case AST_ARGDEF:
                break;
            case AST_STR:
                if (node->res_type == NULL) {
                    node->res_type = (Type*)createPointerType(&context->types,
                        (Type*)createSizedPrimitiveType(&context->types, TYPE_UINT, 8)
                    );
                }
                break;
            case AST_INT:
                if (node->res_type == NULL) {
                    node->res_type = (Type*)createSizedPrimitiveType(&context->types, TYPE_INT, 64);
                }
                break;
            case AST_REAL:
                if (node->res_type == NULL) {
                    node->res_type = (Type*)createSizedPrimitiveType(&context->types, TYPE_REAL, 64);
                }
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
            case AST_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, n->right);
                assumeAmbiguousTypes(context, n->left);
                break;
            }
            case AST_INDEX:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_ADD:
            case AST_OR:
            case AST_AND:
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, n->left);
                assumeAmbiguousTypes(context, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                assumeAmbiguousTypes(context, n->op);
                break;
            }
            case AST_RETURN: {
                AstUnary* n = (AstUnary*)node;
                assumeAmbiguousTypes(context, n->op);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    assumeAmbiguousTypes(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                assumeAmbiguousTypes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                assumeAmbiguousTypes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                assumeAmbiguousTypes(context, n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                assumeAmbiguousTypes(context, n->condition);
                assumeAmbiguousTypes(context, n->if_block);
                assumeAmbiguousTypes(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                assumeAmbiguousTypes(context, n->condition);
                assumeAmbiguousTypes(context, n->block);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                assumeAmbiguousTypes(context, (AstNode*)n->arguments);
                assumeAmbiguousTypes(context, n->body);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                assumeAmbiguousTypes(context, n->function);
                assumeAmbiguousTypes(context, (AstNode*)n->arguments);
                break;
            }
        }
    }
}

static void checkTypes(CompilerContext* context, AstNode* node) {
    // TODO:
    //  - Check not inferred variable types
    //  - Check not inferred node types
    //  - Check constraints (e.g. int for bor/band/bxor, pointer for deref...)
}

#define FOR_ALL_MODULES(ACTION)                                 \
    for (size_t i = 0; i < context->files.file_count; i++) {    \
        File* file = context->files.files[i];                   \
        if (file->ast != NULL) ACTION                           \
    }                                                           \

void runTypeChecking(CompilerContext* context) {
    FOR_ALL_MODULES({ evaluateTypeHints(context, file->ast); });
    FOR_ALL_MODULES({ diffuseTypesOnAllNodes(context, file->ast); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, file->ast); });
    FOR_ALL_MODULES({ diffuseTypesOnAllNodes(context, file->ast); });
    if (context->msgs.error_count == 0) {
        FOR_ALL_MODULES({ checkTypes(context, file->ast); });
    }
}

