
#include "ast/ast.h"
#include "ast/astprinter.h"
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "files/file.h"
#include "text/format.h"
#include "util/alloc.h"

#include "compiler/typecheck.h"

static void raiseConflictingTypes(CompilerContext* context, AstNode* node, Type* other, AstNode* other_reason) {
    String fst_type = buildTypeName(node->res_type);
    String snd_type = buildTypeName(other);
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
    if (other_reason != NULL) {
        notes[note_count] = createMessageFragment(MESSAGE_NOTE,
            createFormattedString("note: expecting `%S` because of this", snd_type),
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
                UNREACHABLE("should not evaluate");
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
                        for (size_t i = 0; i < var->ref_count; i++) {
                            if (n != var->refs[i]) {
                                propagateTypes(context, (AstNode*)var->refs[i], changed);
                            }
                        }
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
                            createFormattedString("type error, definition of invalid type `%S`", type_name), 1,
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

static void propagateToVariableReferences(CompilerContext* context, AstVar* node) {
    SymbolVariable* var = (SymbolVariable*)node->binding;
    if (var != NULL) {
        var->type = node->res_type;
        var->type_reasoning = node->res_type_reasoning;
        for (size_t i = 0; i < var->ref_count; i++) {
            var->refs[i]->res_type = node->res_type;
            var->refs[i]->res_type_reasoning = node->res_type_reasoning;
        }
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
                        createFormattedString("type error, cast to invalid type `%S`", type_name), 1,
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
                        createFormattedString("type error, sizeof for invalid type `%S`", type_name), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->op->location)
                    ));
                    freeString(type_name);
                    n->op->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->op->res_type_reasoning = NULL;
                } else if (!isSizedType(n->op->res_type)) {
                    String type_name = buildTypeName(n->op->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNSIZED_TYPE,
                        createFormattedString("type error, sizeof for unsized type `%S`", type_name), 1,
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
                            createFormattedString("type error, variable has invalid type `%S`", type_name), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                        n->name->res_type_reasoning = NULL;
                    }
                    propagateToVariableReferences(context, n->name);
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
                        createFormattedString("type error, invalid function return type `%S`", type_name), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->ret_type->location)
                    ));
                    freeString(type_name);
                    n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                    n->name->res_type_reasoning = NULL;
                }
                propagateToVariableReferences(context, n->name);
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
                            createFormattedString("type error, variable has invalid type `%S`", type_name), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                        n->name->res_type_reasoning = NULL;
                    }
                    propagateToVariableReferences(context, n->name);
                }
                break;
            }
        }
    }
}

static void propagateVariableReferences(CompilerContext* context, AstVar* node, bool* changed) {
    SymbolVariable* var = (SymbolVariable*)node->binding;
    if (var != NULL) {
        for (size_t i = 0; i < var->ref_count; i++) {
            propagateTypes(context, var->refs[i]->parent, changed);
        }
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
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_UINT, 64);
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
                    Type* type = createSizedPrimitiveType(&context->types, TYPE_UINT, 64);
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
                    if (propagateTypeIntoAstNode(context, (AstNode*)n->name, type, (AstNode*)n->name, changed)) {
                        propagateTypes(context, (AstNode*)n->name, changed);
                    }
                    if (n->val != NULL) {
                        if (propagateTypeIntoAstNode(context, n->val, type, (AstNode*)n->name, changed)) {
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

static void checkForUntypedVariables(CompilerContext* context, AstNode* node) {
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
                checkForUntypedVariables(context, n->right);
                checkForUntypedVariables(context, n->left);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                checkForUntypedVariables(context, n->left);
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
                checkForUntypedVariables(context, n->left);
                checkForUntypedVariables(context, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                checkForUntypedVariables(context, n->op);
                break;
            }
            case AST_SIZEOF:
                break;
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                checkForUntypedVariables(context, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    checkForUntypedVariables(context, field->field_value);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    checkForUntypedVariables(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                checkForUntypedVariables(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                checkForUntypedVariables(context, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                checkForUntypedVariables(context, n->condition);
                checkForUntypedVariables(context, n->if_block);
                checkForUntypedVariables(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                checkForUntypedVariables(context, n->condition);
                checkForUntypedVariables(context, n->block);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                checkForUntypedVariables(context, n->function);
                checkForUntypedVariables(context, (AstNode*)n->arguments);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (n->name->res_type == NULL) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNINFERRED_TYPE,
                        createFormattedString("type error, unable to infer the type of function `%s`", n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("unable to infer the type of this function"), n->name->location)
                    ));
                }
                checkForUntypedVariables(context, (AstNode*)n->arguments);
                checkForUntypedVariables(context, n->body);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                if (n->name->res_type == NULL) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNINFERRED_TYPE,
                        createFormattedString("type error, unable to infer the type of variable `%s`", n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("unable to infer the type of this variable"), n->name->location)
                    ));
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (isPartialType(n->name->res_type)) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNINFERRED_TYPE,
                        createFormattedString("type error, unable to infer the type of variable `%s`", n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("unable to infer the type of this variable"), n->name->location)
                    ));
                    fillPartialType(n->name->res_type, createUnsizedPrimitiveType(&context->types, TYPE_ERROR));
                }
                checkForUntypedVariables(context, n->val);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                checkForUntypedVariables(context, n->strct);
                break;
            }
        }
    }
}

static void checkForUntypedNodes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        if (node->res_type == NULL) {
            addMessageToContext(&context->msgs, createMessage(ERROR_UNINFERRED_TYPE,
                copyFromCString("type error, unable to infer the type of an expression"), 1,
                createMessageFragment(MESSAGE_ERROR, copyFromCString("unable to infer the type of this expression"), node->location)
            ));
        } else {
            switch (node->kind) {
                case AST_STRUCT_TYPE:
                case AST_ARRAY:
                case AST_FN_TYPE:
                    UNREACHABLE("should not evaluate");
                case AST_ERROR:
                case AST_TYPEDEF:
                case AST_ARGDEF:
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
                    checkForUntypedNodes(context, n->right);
                    checkForUntypedNodes(context, n->left);
                    break;
                }
                case AST_AS: {
                    AstBinary* n = (AstBinary*)node;
                    checkForUntypedNodes(context, n->left);
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
                    checkForUntypedNodes(context, n->left);
                    checkForUntypedNodes(context, n->right);
                    break;
                }
                case AST_POS:
                case AST_NEG:
                case AST_ADDR:
                case AST_NOT:
                case AST_DEREF: {
                    AstUnary* n = (AstUnary*)node;
                    checkForUntypedNodes(context, n->op);
                    break;
                }
                case AST_SIZEOF:
                    break;
                case AST_RETURN: {
                    AstReturn* n = (AstReturn*)node;
                    checkForUntypedNodes(context, n->value);
                    break;
                }
                case AST_STRUCT_LIT: {
                    AstList* n = (AstList*)node;
                    for (size_t i = 0; i < n->count; i++) {
                        AstStructField* field = (AstStructField*)n->nodes[i];
                        checkForUntypedNodes(context, field->field_value);
                    }
                    break;
                }
                case AST_ARRAY_LIT:
                case AST_LIST: {
                    AstList* n = (AstList*)node;
                    for (size_t i = 0; i < n->count; i++) {
                        checkForUntypedNodes(context, n->nodes[i]);
                    }
                    break;
                }
                case AST_ROOT: {
                    AstRoot* n = (AstRoot*)node;
                    checkForUntypedNodes(context, (AstNode*)n->nodes);
                    break;
                }
                case AST_BLOCK: {
                    AstBlock* n = (AstBlock*)node;
                    checkForUntypedNodes(context, (AstNode*)n->nodes);
                    break;
                }
                case AST_IF_ELSE: {
                    AstIfElse* n = (AstIfElse*)node;
                    checkForUntypedNodes(context, n->condition);
                    checkForUntypedNodes(context, n->if_block);
                    checkForUntypedNodes(context, n->else_block);
                    break;
                }
                case AST_WHILE: {
                    AstWhile* n = (AstWhile*)node;
                    checkForUntypedNodes(context, n->condition);
                    checkForUntypedNodes(context, n->block);
                    break;
                }
                case AST_CALL: {
                    AstCall* n = (AstCall*)node;
                    checkForUntypedNodes(context, n->function);
                    checkForUntypedNodes(context, (AstNode*)n->arguments);
                    break;
                }
                case AST_FN: {
                    AstFn* n = (AstFn*)node;
                    checkForUntypedNodes(context, (AstNode*)n->arguments);
                    checkForUntypedNodes(context, n->body);
                    break;
                }
                case AST_VARDEF: {
                    AstVarDef* n = (AstVarDef*)node;
                    checkForUntypedNodes(context, n->val);
                    break;
                }
                case AST_STRUCT_INDEX: {
                    AstStructIndex* n = (AstStructIndex*)node;
                    checkForUntypedNodes(context, n->strct);
                    break;
                }
            }
        }
    }
}

static void raiseLiteralTypeError(CompilerContext* context, AstNode* node, const char* kind) {
    String actual_type = buildTypeName(node->res_type);
    String message = createFormattedString(
        "type error, expecting expression of type `%S` but found %s", actual_type, kind
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("%ss are not of type `%S`", kind, actual_type),
        node->location
    );
    if (node->res_type_reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    createFormattedString("note: expecting `%S` because of this", actual_type),
                    node->res_type_reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error)
        );
    }
    freeString(actual_type);
}

static void raiseOpTypeErrorWithHelp(
    CompilerContext* context, AstNode* node, AstNode* err_node, Type* type, AstNode* reasoning,
    const char* hint, const char* help, Span help_location
) {
    String type_name = buildTypeName(type);
    String message = createFormattedString(
        "type error, incompatible type `%S` for %s expession%s", type_name, getAstPrintName(node->kind), hint
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("`%S` type not allowed here", type_name), err_node->location
    );
    MessageFragment* frags[2];
    size_t frag_count = 0;
    if (reasoning != NULL) {
        frags[frag_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: type `%S` defined here", type_name), reasoning->location
        );
        frag_count++;
    }
    if (help != NULL) {
        frags[frag_count] = createMessageFragment(MESSAGE_HELP, copyFromCString(help), help_location);
        frag_count++;
    }
    if (frag_count == 0) {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error)
        );
    } else if (frag_count == 1) {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 2, error, frags[0])
        );
    } else {
        addMessageToContext(
            &context->msgs,
            createMessage(ERROR_INCOMPATIBLE_TYPE, message, 3, error, frags[0], frags[1])
        );
    }
    freeString(type_name);
}

static void raiseOpTypeError(
    CompilerContext* context, AstNode* node, AstNode* err_node, Type* type, AstNode* reasoning, const char* hint
) {
    raiseOpTypeErrorWithHelp(context, node, err_node, type, reasoning, hint, NULL, invalidSpan());
}

static void raiseArgCountError(CompilerContext* context, TypeFunction* type, AstList* arguments, AstNode* reasoning) {
    String message = createFormattedString(
        "too %s arguments to call expression, expected %zi, found %zi",
        type->arg_count < arguments->count ? "many" : "few",
        type->arg_count, arguments->count
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("expected %zi argument%s", type->arg_count, (type->arg_count != 1 ? "s" : "")), arguments->location
    );
    if (reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ARGUMENT_COUNT, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: function defined here"),
                    reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_ARGUMENT_COUNT, message, 1, error)
        );
    }
}

static void raiseVoidReturnError(CompilerContext* context, AstReturn* node, Type* type, AstNode* reasoning) {
    String type_name = buildTypeName(type);
    String message = createFormattedString("type error, expected a return value of type `%S`", type_name);
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("should return value of type `%S`", type_name),
        node->location
    );
    if (reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    createFormattedString("note: expecting `%S` because of this", type_name),
                    reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error)
        );
    }
    freeString(type_name);
}

static void raiseUnsupportedCast(
    CompilerContext* context, AstNode* node, Type* from, Type* to, AstNode* from_reason, AstNode* to_reason
) {
    String fst_type = buildTypeName(from);
    String snd_type = buildTypeName(to);
    String message =
        createFormattedString("type error, unsupported cast form `%S` to `%S`", fst_type, snd_type);
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR,
        createFormattedString(
            "unsupported cast from `%S` to `%S`", fst_type, snd_type
        ),
        node->location
    );
    MessageFragment* notes[2];
    size_t note_count = 0;
    if (from_reason != NULL) {
        notes[note_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: expecting `%S` because of this", fst_type),
            from_reason->location
        );
        note_count++;
    }
    if (to_reason != NULL) {
        notes[note_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: expecting `%S` because of this", snd_type),
            to_reason->location
        );
        note_count++;
    }
    if (note_count == 0) {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error)
        );
    } else if (note_count == 1) {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 2, error, notes[0])
        );
    } else {
        addMessageToContext(
            &context->msgs,
            createMessage(ERROR_INCOMPATIBLE_TYPE, message, 3, error, notes[0], notes[1])
        );
    }
    freeString(fst_type);
    freeString(snd_type);
    node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
    node->res_type_reasoning = NULL;
}

static void raiseNoSuchFieldError(CompilerContext* context, AstStructIndex* node) {
    String type_name = buildTypeName(node->strct->res_type);
    String message = createFormattedString(
        "no field with name `%s` in struct type `%S`", node->field->name, type_name
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("no such field exists"), node->field->location
    );
    if (node->strct->res_type_reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_NO_SUCH_FIELD, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: struct type defined here"),
                    node->strct->res_type_reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_NO_SUCH_FIELD, message, 1, error)
        );
    }
    freeString(type_name);
}

void raiseStructFieldMismatchError(CompilerContext* context, AstNode* node) {
    // TODO: say what is different?
    String type_name = buildTypeName(node->res_type);
    String message = createFormattedString(
        "inconsistent fields in struct literal, expected literal of type `%S`", type_name
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("mismatch in struct fields"), node->location
    );
    if (node->res_type_reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_NO_SUCH_FIELD, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: struct type defined here"),
                    node->res_type_reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_NO_SUCH_FIELD, message, 1, error)
        );
    }
    freeString(type_name);
}

void raiseArrayLengthMismatchError(CompilerContext* context, AstNode* node) {
    String type_name = buildTypeName(node->res_type);
    String message = createFormattedString(
        "inconsistent length of array literal, expected literal of type `%S`", type_name
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("mismatch in array lengths"), node->location
    );
    if (node->res_type_reasoning != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: array type defined here"),
                    node->res_type_reasoning->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_INCOMPATIBLE_TYPE, message, 1, error)
        );
    }
    freeString(type_name);
}

static bool isAddressableValue(AstNode* node) {
    if (node == NULL) {
        return false;
    } else if (node->kind == AST_VAR || node->kind == AST_DEREF) {
        return true;
    } else if (node->kind == AST_INDEX) {
        AstBinary* n = (AstBinary*)node;
        if (isPointerType(n->left->res_type) != NULL) {
            return true;
        } else {
            return isAddressableValue(n->left);
        }
    } else if (node->kind == AST_STRUCT_INDEX) {
        AstStructIndex* n = (AstStructIndex*)node;
        return isAddressableValue(n->strct);
    } else {
        return false;
    }
}

static void checkNodeIsAddressable(CompilerContext* context, AstNode* node) {
    if (!isAddressableValue(node)) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE,
                copyFromCString("attempting to take pointer to expression that is not addressable"), 1,
                createMessageFragment(MESSAGE_ERROR, copyFromCString("expected this to be addressable"), node->location)
            )
        );
    }
}

static bool isNodeWritable(AstNode* node) {
    return isAddressableValue(node) && isSizedType(node->res_type);
}

static void checkNodeIsWritable(CompilerContext* context, AstNode* node) {
    if (!isNodeWritable(node)) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE,
                copyFromCString("the left side of an assignment is not writable"), 1,
                createMessageFragment(MESSAGE_ERROR, copyFromCString("expected this to be writable"), node->location)
            )
        );
    }
}

static void checkTypeConstraints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_ARRAY:
            case AST_FN_TYPE:
                UNREACHABLE("should not evaluate");
            case AST_ERROR:
            case AST_TYPEDEF:
            case AST_VAR:
                break;
            case AST_VOID: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (isVoidType(node->res_type) == NULL) {
                        raiseLiteralTypeError(context, node, "`()`");
                    }
                }
                break;
            }
            case AST_STR: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    TypePointer* ptr_type = isPointerType(node->res_type);
                    TypeSizedPrimitive* int_type = NULL;
                    if (ptr_type != NULL) {
                        int_type = isUnsignedIntegerType(ptr_type->base);
                    }
                    if (int_type == NULL || int_type->size != 8) {
                        raiseLiteralTypeError(context, node, "string literal");
                    }
                }
                break;
            }
            case AST_CHAR:
            case AST_INT: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    TypeSizedPrimitive* int_type = isIntegerType(node->res_type);
                    if (int_type == NULL) {
                        raiseLiteralTypeError(context, node, node->kind == AST_INT ? "integer literal" : "character literal");
                    }
                }
                break;
            }
            case AST_BOOL: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (isBooleanType(node->res_type) == NULL) {
                        raiseLiteralTypeError(context, node, "boolean literal");
                    }
                }
                break;
            }
            case AST_SIZEOF: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (isIntegerType(node->res_type) == NULL) {
                        raiseLiteralTypeError(context, node, "sizeof expression");
                    }
                }
                break;
            }
            case AST_REAL: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    TypeSizedPrimitive* real_type = isRealType(node->res_type);
                    if (real_type == NULL) {
                        raiseLiteralTypeError(context, node, "real literal");
                    }
                }
                break;
            }
            case AST_ADD_ASSIGN:
            case AST_SUB_ASSIGN:
            case AST_MUL_ASSIGN:
            case AST_DIV_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    TypeSizedPrimitive* type = isNumericType(n->left->res_type);
                    if (type == NULL) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, n->left->res_type_reasoning, ", must be a numeric value");
                    }
                }
                checkNodeIsWritable(context, n->left);
                checkTypeConstraints(context, n->right);
                checkTypeConstraints(context, n->left);
                break;
            }
            case AST_MOD_ASSIGN:
            case AST_SHL_ASSIGN:
            case AST_SHR_ASSIGN:
            case AST_BAND_ASSIGN:
            case AST_BOR_ASSIGN:
            case AST_BXOR_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    TypeSizedPrimitive* type = isIntegerType(n->left->res_type);
                    if (type == NULL) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, n->left->res_type_reasoning, ", must be an integer value");
                    }
                }
                checkNodeIsWritable(context, n->left);
                checkTypeConstraints(context, n->right);
                checkTypeConstraints(context, n->left);
                break;
            }
            case AST_ASSIGN: {
                AstBinary* n = (AstBinary*)node;
                checkNodeIsWritable(context, n->left);
                checkTypeConstraints(context, n->right);
                checkTypeConstraints(context, n->left);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && n->res_type != NULL && !compareStructuralTypes(n->left->res_type, n->res_type)) {
                    if (isIntegerType(n->res_type) != NULL) {
                        if (
                            isIntegerType(n->left->res_type) == NULL
                            && isPointerType(n->left->res_type) == NULL
                            && isRealType(n->left->res_type) == NULL
                        ) {
                            raiseUnsupportedCast(
                                context, node, n->left->res_type, n->res_type,
                                n->left->res_type_reasoning, n->res_type_reasoning
                            );
                        }
                    } else if (isRealType(n->res_type) != NULL) {
                        if (isRealType(n->left->res_type) == NULL && isIntegerType(n->left->res_type) == NULL) {
                            raiseUnsupportedCast(
                                context, node, n->left->res_type, n->res_type,
                                n->left->res_type_reasoning, n->res_type_reasoning
                            );
                        }
                    } else if (isPointerType(n->res_type) != NULL) {
                        if (isPointerType(n->left->res_type) == NULL && isIntegerType(n->left->res_type) == NULL) {
                            raiseUnsupportedCast(
                                context, node, n->left->res_type, n->res_type,
                                n->left->res_type_reasoning, n->res_type_reasoning
                            );
                        }
                    } else {
                        raiseUnsupportedCast(
                            context, node, n->left->res_type, n->res_type,
                            n->left->res_type_reasoning, n->res_type_reasoning
                        );
                    }
                }
                checkTypeConstraints(context, n->left);
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    if (isArrayType(n->left->res_type) == NULL && isPointerType(n->left->res_type) == NULL) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, n->left->res_type_reasoning, ", must be an array or pointer");
                    }
                }
                if (n->right->res_type != NULL && !isErrorType(n->right->res_type) && isIntegerType(n->right->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->right, n->right->res_type, n->right->res_type_reasoning, ", must be an integer value");
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && isNumericType(n->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->left, n->res_type, n->res_type_reasoning, ", must be a numeric value");
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_MOD:
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR: {
                AstBinary* n = (AstBinary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && isIntegerType(n->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->left, n->res_type, n->res_type_reasoning, ", must be an integer value");
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_EQ:
            case AST_NE: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    if (isNumericType(n->left->res_type) == NULL && isBooleanType(n->left->res_type) == NULL && isPointerType(n->left->res_type) == NULL) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, n->left->res_type_reasoning, ", must be a numeric value, boolean or pointer");
                    }
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    if (isNumericType(n->left->res_type) == NULL && isPointerType(n->left->res_type) == NULL) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, n->left->res_type_reasoning, ", must be a numeric value or pointer");
                    }
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_POS: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && isNumericType(n->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->op, n->res_type, n->res_type_reasoning, ", must be a numeric value");
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_NEG: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && isRealType(n->res_type) == NULL && isSignedIntegerType(n->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->op, n->res_type, n->res_type_reasoning, ", must be a signed numeric value");
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_NOT: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && isIntegerType(n->res_type) == NULL && isBooleanType(n->res_type) == NULL) {
                    raiseOpTypeError(context, node, n->op, n->res_type, n->res_type_reasoning, ", must be an integer or boolean value");
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (isPointerType(node->res_type) == NULL) {
                        raiseOpTypeError(context, node, node, node->res_type, node->res_type_reasoning, ", always returns a pointer");
                    }
                }
                checkNodeIsAddressable(context, n->op);
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL && !isErrorType(n->op->res_type)) {
                    if (isPointerType(n->op->res_type) == NULL) {
                        raiseOpTypeError(context, node, n->op, n->op->res_type, n->op->res_type_reasoning, ", must be a pointer");
                    }
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                if (n->function->name->res_type != NULL && !isErrorType(n->function->name->res_type)) {
                    TypeFunction* func = isFunctionType(n->function->name->res_type);
                    if (func != NULL) {
                        AstNode* type_reason = n->function->name->res_type_reasoning;
                        if (type_reason != NULL && type_reason->kind == AST_FN) {
                            AstFn* fn_node = (AstFn*)type_reason;
                            if (fn_node->ret_type == NULL) {
                                type_reason = (AstNode*)fn_node->name;
                            } else {
                                type_reason = fn_node->ret_type;
                            }
                        }
                        if (n->value == NULL && isVoidType(func->ret_type) == NULL) {
                            raiseVoidReturnError(context, n, func->ret_type, type_reason);
                        }
                    }
                }
                checkTypeConstraints(context, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type)) {
                    TypeStruct* type = isStructType(n->res_type);
                    if (type != NULL) {
                        sortStructFieldsByName(n);
                        if (type->count != n->count) {
                            raiseStructFieldMismatchError(context, node);
                        } else {
                            for (size_t i = 0; i < type->count; i++) {
                                AstStructField* field = (AstStructField*)n->nodes[i];
                                if (type->names[i] != field->name->name) {
                                    raiseStructFieldMismatchError(context, node);
                                    break;
                                }
                            }
                        }
                    } else {
                        raiseLiteralTypeError(context, node, "struct literal");
                    }
                }
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    checkTypeConstraints(context, field->field_value);
                }
                break;
            }
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type)) {
                    TypeArray* type = isArrayType(n->res_type);
                    if (type != NULL) {
                        if (type->size != n->count) {
                            raiseArrayLengthMismatchError(context, node);
                        }
                    } else {
                        raiseLiteralTypeError(context, node, "array literal");
                    }
                }
                for (size_t i = 0; i < n->count; i++) {
                    checkTypeConstraints(context, n->nodes[i]);
                }
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    checkTypeConstraints(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                checkTypeConstraints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                checkTypeConstraints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                checkTypeConstraints(context, n->condition);
                checkTypeConstraints(context, n->if_block);
                checkTypeConstraints(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                checkTypeConstraints(context, n->condition);
                checkTypeConstraints(context, n->block);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                if (n->function->res_type != NULL && !isErrorType(n->function->res_type)) {
                    TypeFunction* type = isFunctionType(n->function->res_type);
                    if (type == NULL) {
                        TypePointer* ptr_type = isPointerType(n->function->res_type);
                        if (ptr_type != NULL && isFunctionType(ptr_type->base)) {
                            raiseOpTypeErrorWithHelp(
                                context, node, n->function, n->function->res_type,
                                n->function->res_type_reasoning, ", must be a function",
                                "help: consider dereferencing before the function call, e.g. `(&_)(..)`", invalidSpan()
                            );
                        } else {
                            raiseOpTypeError(
                                context, node, n->function, n->function->res_type,
                                n->function->res_type_reasoning, ", must be a function"
                            );
                        }
                    } else if (type->arg_count != n->arguments->count && (!type->vararg || type->arg_count > n->arguments->count)) {
                        raiseArgCountError(context, type, n->arguments, n->function->res_type_reasoning);
                    }
                }
                checkNodeIsAddressable(context, n->function);
                checkTypeConstraints(context, n->function);
                checkTypeConstraints(context, (AstNode*)n->arguments);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                checkTypeConstraints(context, (AstNode*)n->arguments);
                checkTypeConstraints(context, n->body);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                if (n->name->res_type != NULL && !isErrorType(n->name->res_type) && !isSizedType(n->name->res_type)) {
                    String type_name = buildTypeName(n->name->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, unsized type `%S` for variable `%s`", type_name, n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("variable with unsized type"), n->name->location)
                    ));
                    freeString(type_name);
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                checkTypeConstraints(context, n->val);
                if (n->name->res_type != NULL && !isErrorType(n->name->res_type) && !isSizedType(n->name->res_type)) {
                    String type_name = buildTypeName(n->name->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, unsized type `%S` for variable `%s`", type_name, n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("variable with unsized type"), n->name->location)
                    ));
                    freeString(type_name);
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                checkTypeConstraints(context, n->strct);
                if (n->strct->res_type != NULL && !isErrorType(n->strct->res_type)) {
                    TypeStruct* type = isStructType(n->strct->res_type);
                    if (type == NULL) {
                        raiseOpTypeError(
                            context, node, n->strct, n->strct->res_type,
                            n->strct->res_type_reasoning, ", must be a struct"
                        );
                    } else if (lookupIndexOfStructField(type, n->field->name) == NO_POS) {
                        raiseNoSuchFieldError(context, n);
                    }
                }
                break;
            }
        }
    }
}

#define FOR_ALL_MODULES_IF_OK(ACTION) if (context->msgs.error_count == 0) { FOR_ALL_MODULES(ACTION) }

void runTypeChecking(CompilerContext* context) {
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
    FOR_ALL_MODULES_IF_OK({ checkForUntypedVariables(context, file->ast); });
    FOR_ALL_MODULES_IF_OK({ checkTypeConstraints(context, file->ast); });
    FOR_ALL_MODULES_IF_OK({ checkForUntypedNodes(context, file->ast); });
}
