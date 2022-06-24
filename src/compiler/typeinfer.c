
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "compiler/typecheck.h"

#include "compiler/typeinfer.h"

static void raiseConflictingTypes(CompilerContext* context, AstNode* node, Type* other, AstNode* other_reason) {
    String fst_type = buildTypeName(node->res_type);
    String snd_type = buildTypeName(other);
    String message = createFormattedString("type error, conflicting types `%s` and `%s`", cstr(fst_type), cstr(snd_type));
    MessageFragment* error = createMessageFragment(MESSAGE_ERROR,
        createFormattedString("conflicting types `%s` and `%s` for this expression", cstr(fst_type), cstr(snd_type)), node->location
    );
    MessageFragment* notes[2];
    size_t note_count = 0;
    if (node->res_type_reasoning != NULL) {
        notes[note_count] = createMessageFragment(MESSAGE_NOTE,
            createFormattedString("note: expecting `%s` because of this", cstr(fst_type)),
            node->res_type_reasoning->location
        );
        note_count++;
    }
    if (other_reason != NULL) {
        notes[note_count] = createMessageFragment(MESSAGE_NOTE,
            createFormattedString("note: expecting `%s` because of this", cstr(snd_type)),
            other_reason->location
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
}

static bool propagateTypeIntoAstNode(
    CompilerContext* context, AstNode* node, Type* type, AstNode* reasoning, bool* changed
) {
    bool changed_this = false;
    if (node->res_type == NULL) {
        node->res_type = type;
        node->res_type_reasoning = reasoning;
        *changed = true;
        return true;
    } else if (!assertStructuralTypesEquality(node->res_type, type, &changed_this)) {
        raiseConflictingTypes(context, node, type, reasoning);
        node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
        node->res_type_reasoning = NULL;
        *changed = true;
        return true;
    } else if (changed_this) {
        *changed = true;
        return true;
    } else {
        return false;
    }
}

static bool propagateTypeFromIntoAstNode(CompilerContext* context, AstNode* into, AstNode* from, bool* changed) {
    return propagateTypeIntoAstNode(context, into, from->res_type, from->res_type_reasoning, changed);
}

static AstNode* getStructFieldReasoning(AstNode* type_reason, Symbol name) {
    if (type_reason != NULL && (type_reason->kind == AST_STRUCT_TYPE || type_reason->kind == AST_STRUCT_LIT)) {
        AstList* strct_node = (AstList*)type_reason;
        for (size_t i = 0; i < strct_node->count; i++) {
            AstStructField* field = (AstStructField*)strct_node->nodes[i];
            if (field->name->name == name) {
                return field->type->res_type_reasoning;
            }
        }
    }
    return type_reason;
}

static AstNode* getArrayElementReasoning(AstNode* type_reason) {
    if (type_reason != NULL && type_reason->kind == AST_ARRAY) {
        AstBinary* arr_node = (AstBinary*)type_reason;
        return arr_node->right;
    } else if (type_reason != NULL && type_reason->kind == AST_ARRAY_LIT) {
        AstList* arr_node = (AstList*)type_reason;
        if (arr_node->count > 0) {
            return arr_node->nodes[0]->res_type_reasoning;
        }
    }
    return type_reason;
}

static AstNode* getPointerElementReasoning(AstNode* type_reason) {
    if (type_reason != NULL && type_reason->kind == AST_ADDR) {
        AstUnary* addr_node = (AstUnary*)type_reason;
        return addr_node->op->res_type_reasoning;
    }
    return type_reason;
}

static AstNode* getFunctionReturnReasoning(AstNode* type_reason) {
    if (type_reason != NULL && type_reason->kind == AST_FN) {
        AstFn* fn_node = (AstFn*)type_reason;
        if (fn_node->ret_type == NULL) {
            return (AstNode*)fn_node->name;
        } else {
            return fn_node->ret_type;
        }
    } else if (type_reason != NULL && type_reason->kind == AST_FN_TYPE) {
        AstFnType* fn_node = (AstFnType*)type_reason;
        if (fn_node->ret_type == NULL) {
            return (AstNode*)fn_node;
        } else {
            return fn_node->ret_type;
        }
    }
    return type_reason;
}

static AstNode* getFunctionParamReasoning(AstNode* type_reason, size_t i) {
    if (type_reason != NULL && type_reason->kind == AST_FN) {
        AstFn* fn_node = (AstFn*)type_reason;
        AstArgDef* arg_node = (AstArgDef*)fn_node->arguments->nodes[i];
        if (arg_node->type == NULL) {
            return (AstNode*)arg_node;
        } else {
            return arg_node->type;
        }
    } else if (type_reason != NULL && type_reason->kind == AST_FN_TYPE) {
        AstFnType* fn_node = (AstFnType*)type_reason;
        return fn_node->arguments->nodes[i];
    }
    return type_reason;
}

static void propagateTypes(CompilerContext* context, AstNode* node, bool* changed) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_FN_TYPE:
            case AST_ARRAY:
                break; // Are reachable from variable propagations
            case AST_ARGDEF:
                propagateTypes(context, node->parent, changed);
                break;
            case AST_ERROR:
            case AST_TYPEDEF:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_CHAR:
            case AST_REAL:
            case AST_BOOL:
            case AST_ROOT:
            case AST_BLOCK:
            case AST_IF_ELSE:
            case AST_WHILE:
            case AST_FN:
                break;
            case AST_LIST: {
                propagateTypes(context, node->parent, changed);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var != NULL) {
                    if (var->type != NULL) {
                        if (propagateTypeIntoAstNode(context, node, var->type, var->type_reasoning, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                    } else if (n->res_type != NULL) {
                        var->type = n->res_type;
                        var->type_reasoning = n->res_type_reasoning;
                        FOR_ALL_VAR_REFS(var, {
                            if (n != ref) {
                                propagateTypes(context, (AstNode*)ref, changed);
                            }
                        })
                        propagateTypes(context, (AstNode*)var->def, changed);
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
                    if (propagateTypeFromIntoAstNode(context, n->left, n->right, changed)) {
                        propagateTypes(context, n->left, changed);
                    }
                }
                if (n->left->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, n->right, n->left, changed)) {
                        propagateTypes(context, n->right, changed);
                    }
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL) {
                    if (n->name->res_type != NULL) {
                        if (propagateTypeFromIntoAstNode(context, n->val, (AstNode*)n->name, changed)) {
                            propagateTypes(context, n->val, changed);
                        }
                    }
                    if (n->val->res_type != NULL) {
                        if (propagateTypeFromIntoAstNode(context, (AstNode*)n->name, n->val, changed)) {
                            propagateTypes(context, (AstNode*)n->name, changed);
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
                    if (propagateTypeFromIntoAstNode(context, n->left, node, changed)) {
                        propagateTypes(context, n->left, changed);
                    }
                    if (propagateTypeFromIntoAstNode(context, n->right, node, changed)) {
                        propagateTypes(context, n->right, changed);
                    }
                }
                if (n->left->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, node, n->left, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                    if (propagateTypeFromIntoAstNode(context, n->right, n->left, changed)) {
                        propagateTypes(context, n->right, changed);
                    }
                }
                if (n->right->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, node, n->right, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                    if (propagateTypeFromIntoAstNode(context, n->left, n->right, changed)) {
                        propagateTypes(context, n->left, changed);
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
                    if (propagateTypeFromIntoAstNode(context, n->right, n->left, changed)) {
                        propagateTypes(context, n->right, changed);
                    }
                }
                if (n->right->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, n->left, n->right, changed)) {
                        propagateTypes(context, n->left, changed);
                    }
                }
                break;
            }
            case AST_AS:
                break;
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL) {
                    TypeArray* arr_type = isArrayType(n->left->res_type);
                    TypePointer* ptr_type = isPointerType(n->left->res_type);
                    if (arr_type != NULL) {
                        AstNode* type_reason = getArrayElementReasoning(n->left->res_type_reasoning);
                        if (propagateTypeIntoAstNode(context, node, arr_type->base, type_reason, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                    }
                    if (ptr_type != NULL) {
                        AstNode* type_reason = getPointerElementReasoning(n->left->res_type_reasoning);
                        if (propagateTypeIntoAstNode(context, node, ptr_type->base, type_reason, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                    }
                }
                break;
            }
            case AST_OR:
            case AST_AND:
                break;
            case AST_SIZEOF:
                break;
            case AST_NOT:
            case AST_POS:
            case AST_NEG: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, n->op, node, changed)) {
                        propagateTypes(context, n->op, changed);
                    }
                }
                if (n->op->res_type != NULL) {
                    if (propagateTypeFromIntoAstNode(context, node, n->op, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL) {
                    if (propagateTypeIntoAstNode(context, node, createPointerType(&context->types, n->op->res_type), node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                if (n->res_type != NULL) {
                    TypePointer* type = isPointerType(n->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = getPointerElementReasoning(n->res_type_reasoning);
                        if (propagateTypeIntoAstNode(context, n->op, type->base, type_reason, changed)) {
                            propagateTypes(context, n->op, changed);
                        }
                    }
                }
                break;
            }
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL) {
                    if (propagateTypeIntoAstNode(context, n->op, createPointerType(&context->types, n->res_type), node->res_type_reasoning, changed)) {
                        propagateTypes(context, n->op, changed);
                    }
                }
                if (n->op->res_type != NULL) {
                    TypePointer* type = isPointerType(n->op->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = getPointerElementReasoning(n->op->res_type_reasoning);
                        if (propagateTypeIntoAstNode(context, node, type->base, type_reason, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                    }
                }
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                if (n->function->res_type != NULL) {
                    TypeFunction* type = isFunctionType(n->function->res_type);
                    if (
                        type != NULL
                        && (type->arg_count == n->arguments->count
                            || (type->vararg && type->arg_count < n->arguments->count)
                        )
                    ) {
                        AstNode* type_reason = getFunctionReturnReasoning(n->function->res_type_reasoning);
                        if (propagateTypeIntoAstNode(context, node, type->ret_type, type_reason, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                        for (size_t i = 0; i < type->arg_count; i++) {
                            type_reason = getFunctionParamReasoning(n->function->res_type_reasoning, i);
                            if (propagateTypeIntoAstNode(context, n->arguments->nodes[i], type->arguments[i], type_reason, changed)) {
                                propagateTypes(context, n->arguments->nodes[i], changed);
                            }
                        }
                    }
                }
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL) {
                    TypeStruct* type = isStructType(n->res_type);
                    if (type != NULL) {
                        for (size_t i = 0; i < n->count; i++) {
                            AstStructField* field = (AstStructField*)n->nodes[i];
                            size_t idx = lookupIndexOfStructField(type, field->name->name);
                            if (idx != NO_POS) {
                                AstNode* type_reason = getStructFieldReasoning(n->res_type_reasoning, field->name->name);
                                if (propagateTypeIntoAstNode(context, field->field_value, type->types[idx], type_reason, changed)) {
                                    propagateTypes(context, field->field_value, changed);
                                }
                            }
                        }
                    }
                }
                bool has_all = true;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    if (field->field_value->res_type == NULL) {
                        has_all = false;
                        break;
                    }
                }
                if (has_all) {
                    if (!checkStructFieldsHaveNoDups(context, n)) {
                        Symbol* names = ALLOC(Symbol, n->count);
                        Type** types = ALLOC(Type*, n->count);
                        for (size_t i = 0; i < n->count; i++) {
                            AstStructField* field = (AstStructField*)n->nodes[i];
                            names[i] = field->name->name;
                            types[i] = field->field_value->res_type;
                        }
                        Type* type = createTypeStruct(&context->types, names, types, n->count);
                        if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                            propagateTypes(context, node->parent, changed);
                        }
                    }
                }
                break;
            }
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL) {
                    TypeArray* type = isArrayType(n->res_type);
                    if (type != NULL) {
                        AstNode* type_reason = getArrayElementReasoning(n->res_type_reasoning);
                        for (size_t i = 0; i < n->count; i++) {
                            if (propagateTypeIntoAstNode(context, n->nodes[i], type->base, type_reason, changed)) {
                                propagateTypes(context, n->nodes[i], changed);
                            }
                        }
                    }
                }
                Type* type = NULL;
                AstNode* reasoning = NULL;
                for (size_t i = 0; i < n->count; i++) {
                    if (n->nodes[i]->res_type != NULL) {
                        type = n->nodes[i]->res_type;
                        reasoning = n->nodes[i]->res_type_reasoning;
                        break;
                    }
                }
                if (type != NULL) {
                    for (size_t i = 0; i < n->count; i++) {
                        if (propagateTypeIntoAstNode(context, n->nodes[i], type, reasoning, changed)) {
                            propagateTypes(context, n->nodes[i], changed);
                        }
                    }
                    Type* arr_type = createArrayType(&context->types, type, n->count);
                    if (propagateTypeIntoAstNode(context, node, arr_type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                if (n->function->name->res_type != NULL) {
                    TypeFunction* func = isFunctionType(n->function->name->res_type);
                    if (func != NULL) {
                        if (n->value != NULL) {
                            AstNode* type_reason = getFunctionReturnReasoning(n->function->name->res_type_reasoning);
                            if (propagateTypeIntoAstNode(context, n->value, func->ret_type, type_reason, changed)) {
                                propagateTypes(context, n->value, changed);
                            }
                        }
                    }
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                if (n->strct->res_type != NULL) {
                    TypeStruct* type = isStructType(n->strct->res_type);
                    if (type != NULL) {
                        size_t idx = lookupIndexOfStructField(type, n->field->name);
                        if (idx != NO_POS) {
                            AstNode* type_reason = getStructFieldReasoning(n->strct->res_type_reasoning, n->field->name);
                            if (propagateTypeIntoAstNode(context, node, type->types[idx], type_reason, changed)) {
                                propagateTypes(context, node->parent, changed);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

static void evaluateTypeDefinitions(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    evaluateTypeDefinitions(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                evaluateTypeDefinitions(context, (AstNode*)n->nodes);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                SymbolType* type = (SymbolType*)n->name->binding;
                if (type != NULL && type->type == NULL) {
                    type->type = evaluateTypeExpr(context, n->value);
                }
                n->name->res_type = type->type;
                break;
            }
            default:
                // Type definitions must be global
                break;
        }
    }
}

static void checkTypeDefinitions(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    checkTypeDefinitions(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                checkTypeDefinitions(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                checkTypeDefinitions(context, (AstNode*)n->nodes);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                SymbolType* type = (SymbolType*)n->name->binding;
                if (type != NULL && type->type != NULL) {
                    if (!isValidType(type->type)) {
                        String type_name = buildTypeName(n->name->res_type);
                        addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                            createFormattedString("type error, definition of invalid type `%s`", cstr(type_name)), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->value->location)
                        ));
                        freeString(type_name);
                        type->type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                        n->name->res_type = type->type;
                    }
                }
                break;
            }
            default:
                // Type definitions must be global
                break;
        }
    }
}

static void propagateToVariableReferences(AstVar* node) {
    SymbolVariable* var = (SymbolVariable*)node->binding;
    if (var != NULL) {
        var->type = node->res_type;
        var->type_reasoning = node->res_type_reasoning;
        FOR_ALL_VAR_REFS(var, {
            ref->res_type = node->res_type;
            ref->res_type_reasoning = node->res_type_reasoning;
        })
        var->def->res_type = node->res_type;
        var->def->res_type_reasoning = node->res_type_reasoning;
    }
}

static void evaluateTypeHints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_FN_TYPE:
            case AST_ARRAY: {
                UNREACHABLE("should not evaluate");
            }
            case AST_ERROR: {
                node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                break;
            }
            case AST_VAR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_CHAR:
            case AST_BOOL:
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
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                n->right->res_type = evaluateTypeExpr(context, n->right);
                n->right->res_type_reasoning = n->right;
                if (!isValidType(n->right->res_type)) {
                    String type_name = buildTypeName(n->right->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, cast to invalid type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->right->location)
                    ));
                    freeString(type_name);
                    n->right->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->right->res_type_reasoning = NULL;
                }
                n->res_type = n->right->res_type;
                n->res_type_reasoning = n->right->res_type_reasoning;
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
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                evaluateTypeHints(context, n->strct);
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
            case AST_SIZEOF: {
                AstUnary* n = (AstUnary*)node;
                n->op->res_type = evaluateTypeExpr(context, n->op);
                n->op->res_type_reasoning = n->op;
                if (!isValidType(n->op->res_type)) {
                    String type_name = buildTypeName(n->op->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, sizeof for invalid type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->op->location)
                    ));
                    freeString(type_name);
                    n->op->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->op->res_type_reasoning = NULL;
                } else if (!isSizedType(n->op->res_type)) {
                    String type_name = buildTypeName(n->op->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNSIZED_TYPE,
                        createFormattedString("type error, sizeof for unsized type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is unsized"), n->op->location)
                    ));
                    freeString(type_name);
                    n->op->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->op->res_type_reasoning = NULL;
                }
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                evaluateTypeHints(context, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    evaluateTypeHints(context, field->field_value);
                }
                break;
            }
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    evaluateTypeHints(context, n->nodes[i]);
                }
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
                    n->name->res_type = evaluateTypeExpr(context, n->type);
                    n->name->res_type_reasoning = n->type;
                    if (!isValidType(n->name->res_type)) {
                        String type_name = buildTypeName(n->name->res_type);
                        addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                            createFormattedString("type error, variable has invalid type `%s`", cstr(type_name)), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                        n->name->res_type_reasoning = NULL;
                    }
                    propagateToVariableReferences(n->name);
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
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                evaluateTypeHints(context, n->function);
                evaluateTypeHints(context, (AstNode*)n->arguments);
                break;
            }
            case AST_TYPEDEF:{
                AstTypeDef* n = (AstTypeDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
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
                    arg_types[i] = def->name->res_type;
                }
                n->name->res_type = createFunctionType(
                    &context->types, ret_type, n->arguments->count, arg_types,
                    (n->flags & AST_FN_FLAG_VARARG) != 0
                );
                n->name->res_type_reasoning = node;
                if (!isValidType(ret_type)) {
                    String type_name = buildTypeName(ret_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, invalid function return type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->ret_type->location)
                    ));
                    freeString(type_name);
                    n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->name->res_type_reasoning = NULL;
                }
                propagateToVariableReferences(n->name);
                evaluateTypeHints(context, n->body);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                if (n->type != NULL) {
                    n->name->res_type = evaluateTypeExpr(context, n->type);
                    n->name->res_type_reasoning = (AstNode*)n->type;
                    if (!isValidType(n->name->res_type)) {
                        String type_name = buildTypeName(n->name->res_type);
                        addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                            createFormattedString("type error, variable has invalid type `%s`", cstr(type_name)), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                        n->name->res_type_reasoning = NULL;
                    }
                    propagateToVariableReferences(n->name);
                }
                break;
            }
        }
    }
}

static void propagateVariableReferences(CompilerContext* context, AstVar* node, bool* changed) {
    SymbolVariable* var = (SymbolVariable*)node->binding;
    if (var != NULL) {
        FOR_ALL_VAR_REFS(var, {
            propagateTypes(context, ref->parent, changed);
        })
        propagateTypes(context, var->def->parent, changed);
    }
}

static void propagateAllTypes(CompilerContext* context, AstNode* node, bool* changed) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_ARRAY:
            case AST_FN_TYPE:
                UNREACHABLE("should not evaluate");
            case AST_ERROR:
            case AST_TYPEDEF:
            case AST_VAR:
            case AST_VOID:
            case AST_STR:
            case AST_INT:
            case AST_CHAR:
            case AST_BOOL:
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
                propagateAllTypes(context, n->right, changed);
                propagateAllTypes(context, n->left, changed);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                propagateAllTypes(context, n->left, changed);
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
                propagateAllTypes(context, n->left, changed);
                propagateAllTypes(context, n->right, changed);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                propagateAllTypes(context, n->strct, changed);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                propagateAllTypes(context, n->op, changed);
                break;
            }
            case AST_SIZEOF:
                break;
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                propagateAllTypes(context, n->value, changed);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    propagateAllTypes(context, field->field_value, changed);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    propagateAllTypes(context, n->nodes[i], changed);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                propagateAllTypes(context, (AstNode*)n->nodes, changed);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                propagateAllTypes(context, (AstNode*)n->nodes, changed);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                propagateAllTypes(context, n->condition, changed);
                propagateAllTypes(context, n->if_block, changed);
                propagateAllTypes(context, n->else_block, changed);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                propagateAllTypes(context, n->condition, changed);
                propagateAllTypes(context, n->block, changed);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                propagateAllTypes(context, n->function, changed);
                propagateAllTypes(context, (AstNode*)n->arguments, changed);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                propagateVariableReferences(context, n->name, changed);
                propagateAllTypes(context, (AstNode*)n->arguments, changed);
                propagateAllTypes(context, n->body, changed);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                propagateVariableReferences(context, n->name, changed);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                propagateVariableReferences(context, n->name, changed);
                propagateAllTypes(context, n->val, changed);
                break;
            }
        }
        propagateTypes(context, node, changed);
    }
}

typedef enum {
    ASSUME_NONE = 0,
    ASSUME_CASTS = (1 << 0),
    ASSUME_LITERALS = (1 << 1),
    ASSUME_SIZEOF = (1 << 2),
    ASSUME_INDEX = (1 << 3),
    ASSUME_VARS = (1 << 4),
} AssumeAmbiguousPhase;

static void assumeAmbiguousTypes(CompilerContext* context, AssumeAmbiguousPhase phase, AstNode* node, bool* changed) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_ARRAY:
            case AST_FN_TYPE:
                UNREACHABLE("should not evaluate");
            case AST_ERROR:
            case AST_VAR:
            case AST_TYPEDEF:
            case AST_ARGDEF:
                break;
            case AST_STR:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createPointerType(&context->types,
                        createSizedPrimitiveType(&context->types, TYPE_UINT, 8)
                    );
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_VOID:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_CHAR:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_UINT, 8);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_INT:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_INT, 64);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_BOOL:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_REAL:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_REAL, 64);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_SIZEOF:
                if ((phase & ASSUME_SIZEOF) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_UINT, SIZE_SIZE);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, node, type, node, changed)) {
                        propagateTypes(context, node->parent, changed);
                    }
                }
                break;
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, phase, n->left, changed);
                if ((phase & ASSUME_CASTS) != 0 && n->left->res_type == NULL && n->right->res_type != NULL) {
                    Type* type = n->right->res_type;
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, n->left, type, n->right, changed)) {
                        propagateTypes(context, n->left, changed);
                    }
                }
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, phase, n->left, changed);
                assumeAmbiguousTypes(context, phase, n->right, changed);
                if ((phase & ASSUME_INDEX) != 0 && n->right->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_UINT, SIZE_SIZE);
                    type = createUnsureType(&context->types, type);
                    if (propagateTypeIntoAstNode(context, n->right, type, n->right, changed)) {
                        propagateTypes(context, n->right, changed);
                    }
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                assumeAmbiguousTypes(context, phase, n->val, changed);
                if ((phase & ASSUME_VARS) != 0 && n->name->res_type == NULL) {
                    Type* type = createUnsureType(&context->types, NULL);
                    if (propagateTypeIntoAstNode(context, (AstNode*)n->name, type, NULL, changed)) {
                        propagateTypes(context, (AstNode*)n->name, changed);
                    }
                    if (n->val != NULL) {
                        if (propagateTypeIntoAstNode(context, n->val, type, NULL, changed)) {
                            propagateTypes(context, n->val, changed);
                        }
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
                assumeAmbiguousTypes(context, phase, n->right, changed);
                assumeAmbiguousTypes(context, phase, n->left, changed);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                assumeAmbiguousTypes(context, phase, n->strct, changed);
                break;
            }
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
                assumeAmbiguousTypes(context, phase, n->left, changed);
                assumeAmbiguousTypes(context, phase, n->right, changed);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                assumeAmbiguousTypes(context, phase, n->op, changed);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                assumeAmbiguousTypes(context, phase, n->value, changed);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    assumeAmbiguousTypes(context, phase, field->field_value, changed);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    assumeAmbiguousTypes(context, phase, n->nodes[i], changed);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->nodes, changed);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->nodes, changed);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                assumeAmbiguousTypes(context, phase, n->condition, changed);
                assumeAmbiguousTypes(context, phase, n->if_block, changed);
                assumeAmbiguousTypes(context, phase, n->else_block, changed);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                assumeAmbiguousTypes(context, phase, n->condition, changed);
                assumeAmbiguousTypes(context, phase, n->block, changed);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->arguments, changed);
                assumeAmbiguousTypes(context, phase, n->body, changed);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                assumeAmbiguousTypes(context, phase, n->function, changed);
                assumeAmbiguousTypes(context, phase, (AstNode*)n->arguments, changed);
                break;
            }
        }
    }
}

void typeInferExpr(CompilerContext* context, AstNode* node, Type* assumption) {
    bool changed = false;
    evaluateTypeHints(context, node);
    propagateAllTypes(context, node, &changed); 
    assumeAmbiguousTypes(context, ASSUME_LITERALS, node, &changed);
    assumeAmbiguousTypes(context, ASSUME_CASTS, node, &changed);
    if (assumption != NULL && node->res_type == NULL) {
        Type* type = createUnsureType(&context->types, assumption);
        if (propagateTypeIntoAstNode(context, node, type, node, &changed)) {
            propagateTypes(context, node->parent, &changed);
        }
    }
}

void runTypeInference(CompilerContext* context) {
    bool changed = false;
    FOR_ALL_MODULES({ evaluateTypeDefinitions(context, file->ast); });
    FOR_ALL_MODULES({ checkTypeDefinitions(context, file->ast); });
    FOR_ALL_MODULES({ evaluateTypeHints(context, file->ast); });
    FOR_ALL_MODULES({ propagateAllTypes(context, file->ast, &changed); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_SIZEOF | ASSUME_INDEX, file->ast, &changed); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_LITERALS, file->ast, &changed); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_CASTS, file->ast, &changed); });
    FOR_ALL_MODULES_IF_OK({ checkTypeConstraints(context, file->ast); });
    changed = false;
    FOR_ALL_MODULES_IF_OK({ assumeAmbiguousTypes(context, ASSUME_VARS, file->ast, &changed); });
    while (changed) {
        changed = false;
        FOR_ALL_MODULES({ propagateAllTypes(context, file->ast, &changed); });
    }
}

