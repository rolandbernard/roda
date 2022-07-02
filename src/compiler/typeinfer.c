
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "compiler/typecheck.h"

#include "compiler/typeinfer.h"

static void raiseConflictingTypes(CompilerContext* context, AstNode* node, Type* other) {
    String fst_type = buildTypeName(node->res_type);
    String snd_type = buildTypeName(other);
    String message = createFormattedString("type error, conflicting types `%s` and `%s`", cstr(fst_type), cstr(snd_type));
    MessageFragment* error = createMessageFragment(MESSAGE_ERROR,
        createFormattedString("conflicting types `%s` and `%s` for this expression", cstr(fst_type), cstr(snd_type)), node->location
    );
    MessageFragment* notes[2];
    size_t note_count = 0;
    if (getTypeReason(node->res_type) != NULL) {
        notes[note_count] = createMessageFragment(MESSAGE_NOTE,
            createFormattedString("note: expecting `%s` because of this", cstr(fst_type)),
            getTypeReason(node->res_type)->location
        );
        note_count++;
    }
    if (getTypeReason(other) != NULL) {
        notes[note_count] = createMessageFragment(MESSAGE_NOTE,
            createFormattedString("note: expecting `%s` because of this", cstr(snd_type)),
            getTypeReason(other)->location
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

static void propagateTypes(CompilerContext* context, AstNode* node);

static bool moveTypeIntoAstNode(CompilerContext* context, AstNode* node, Type* type) {
    TypeUnsure* changed_this = NULL;
    if (node->res_type == NULL) {
        node->res_type = type;
        node->type_ref_next = type->refs;
        type->refs = node;
        return true;
    } else if (!assertStructuralTypesEquality(node->res_type, type, &changed_this)) {
        raiseConflictingTypes(context, node, type);
        node->res_type = getErrorType(&context->types);
        return true;
    } else {
        while (changed_this != NULL) {
            AstNode* ref = changed_this->refs;
            while (ref != NULL) {
                propagateTypes(context, ref);
                propagateTypes(context, ref->parent);
                ref = ref->type_ref_next;
            }
            changed_this = changed_this->changed_next;
        }
        return false;
    }
}

static bool moveTypeFromIntoAstNode(CompilerContext* context, AstNode* into, AstNode* from) {
    return moveTypeIntoAstNode(context, into, from->res_type);
}

static void propagateTypes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
            case AST_FN_TYPE:
            case AST_ARRAY:
                break; // Are reachable from variable propagations
            case AST_ARGDEF:
                propagateTypes(context, node->parent);
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
            case AST_BREAK:
            case AST_CONTINUE:
                break;
            case AST_IF_ELSE_EXPR: {
                AstIfElse* n = (AstIfElse*)node;
                if (n->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, n->if_block, node)) {
                        propagateTypes(context, n->if_block);
                    }
                    if (moveTypeFromIntoAstNode(context, n->else_block, node)) {
                        propagateTypes(context, n->else_block);
                    }
                }
                if (n->if_block->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, n->if_block)) {
                        propagateTypes(context, node->parent);
                    }
                    if (moveTypeFromIntoAstNode(context, n->else_block, n->if_block)) {
                        propagateTypes(context, n->else_block);
                    }
                }
                if (n->else_block->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, n->else_block)) {
                        propagateTypes(context, node->parent);
                    }
                    if (moveTypeFromIntoAstNode(context, n->if_block, n->else_block)) {
                        propagateTypes(context, n->if_block);
                    }
                }
                break;
            }
            case AST_BLOCK_EXPR: {
                AstBlock* n = (AstBlock*)node;
                AstNode* last = n->nodes->nodes[n->nodes->count - 1];
                if (n->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, last, node)) {
                        propagateTypes(context, last);
                    }
                }
                if (last->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, last)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            }
            case AST_LIST: {
                propagateTypes(context, node->parent);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var != NULL) {
                    if (var->type != NULL) {
                        if (moveTypeIntoAstNode(context, node, var->type)) {
                            propagateTypes(context, node->parent);
                        }
                    } else if (n->res_type != NULL) {
                        var->type = n->res_type;
                        FOR_ALL_VAR_REFS(var, {
                            if (n != ref) {
                                propagateTypes(context, (AstNode*)ref);
                            }
                        })
                        propagateTypes(context, (AstNode*)var->def);
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
                    if (moveTypeFromIntoAstNode(context, n->left, n->right)) {
                        propagateTypes(context, n->left);
                    }
                }
                if (n->left->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, n->right, n->left)) {
                        propagateTypes(context, n->right);
                    }
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (n->val != NULL) {
                    if (n->name->res_type != NULL) {
                        if (moveTypeFromIntoAstNode(context, n->val, (AstNode*)n->name)) {
                            propagateTypes(context, n->val);
                        }
                    }
                    if (n->val->res_type != NULL) {
                        if (moveTypeFromIntoAstNode(context, (AstNode*)n->name, n->val)) {
                            propagateTypes(context, (AstNode*)n->name);
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
                    if (moveTypeFromIntoAstNode(context, n->left, node)) {
                        propagateTypes(context, n->left);
                    }
                    if (moveTypeFromIntoAstNode(context, n->right, node)) {
                        propagateTypes(context, n->right);
                    }
                }
                if (n->left->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, n->left)) {
                        propagateTypes(context, node->parent);
                    }
                    if (moveTypeFromIntoAstNode(context, n->right, n->left)) {
                        propagateTypes(context, n->right);
                    }
                }
                if (n->right->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, n->right)) {
                        propagateTypes(context, node->parent);
                    }
                    if (moveTypeFromIntoAstNode(context, n->left, n->right)) {
                        propagateTypes(context, n->left);
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
                    if (moveTypeFromIntoAstNode(context, n->right, n->left)) {
                        propagateTypes(context, n->right);
                    }
                }
                if (n->right->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, n->left, n->right)) {
                        propagateTypes(context, n->left);
                    }
                }
                break;
            }
            case AST_AS:
                break;
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL) {
                    TypeArray* arr_type = (TypeArray*)getTypeOfKind(n->left->res_type, TYPE_ARRAY);
                    TypePointer* ptr_type = (TypePointer*)getTypeOfKind(n->left->res_type, TYPE_POINTER);
                    if (arr_type != NULL) {
                        if (moveTypeIntoAstNode(context, node, arr_type->base)) {
                            propagateTypes(context, node->parent);
                        }
                    }
                    if (ptr_type != NULL) {
                        if (moveTypeIntoAstNode(context, node, ptr_type->base)) {
                            propagateTypes(context, node->parent);
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
                    if (moveTypeFromIntoAstNode(context, n->op, node)) {
                        propagateTypes(context, n->op);
                    }
                }
                if (n->op->res_type != NULL) {
                    if (moveTypeFromIntoAstNode(context, node, n->op)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL) {
                    if (moveTypeIntoAstNode(context, node, createPointerType(&context->types, node, n->op->res_type))) {
                        propagateTypes(context, node->parent);
                    }
                }
                if (n->res_type != NULL) {
                    TypePointer* type = (TypePointer*)getTypeOfKind(n->res_type, TYPE_POINTER);
                    if (type != NULL) {
                        if (moveTypeIntoAstNode(context, n->op, type->base)) {
                            propagateTypes(context, n->op);
                        }
                    }
                }
                break;
            }
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL) {
                    if (moveTypeIntoAstNode(context, n->op, createPointerType(&context->types, node, n->res_type))) {
                        propagateTypes(context, n->op);
                    }
                }
                if (n->op->res_type != NULL) {
                    TypePointer* type = (TypePointer*)getTypeOfKind(n->op->res_type, TYPE_POINTER);
                    if (type != NULL) {
                        if (moveTypeIntoAstNode(context, node, type->base)) {
                            propagateTypes(context, node->parent);
                        }
                    }
                }
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                if (n->function->res_type != NULL) {
                    TypeFunction* type = (TypeFunction*)getTypeOfKind(n->function->res_type, TYPE_FUNCTION);
                    if (
                        type != NULL
                        && (type->arg_count == n->arguments->count
                            || (type->vararg && type->arg_count < n->arguments->count)
                        )
                    ) {
                        if (moveTypeIntoAstNode(context, node, type->ret_type)) {
                            propagateTypes(context, node->parent);
                        }
                        for (size_t i = 0; i < type->arg_count; i++) {
                            if (moveTypeIntoAstNode(context, n->arguments->nodes[i], type->arguments[i])) {
                                propagateTypes(context, n->arguments->nodes[i]);
                            }
                        }
                    }
                }
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL) {
                    TypeStruct* type = (TypeStruct*)getTypeOfKind(n->res_type, TYPE_STRUCT);
                    if (type != NULL) {
                        for (size_t i = 0; i < n->count; i++) {
                            AstStructField* field = (AstStructField*)n->nodes[i];
                            size_t idx = lookupIndexOfStructField(type, field->name->name);
                            if (idx != NO_POS) {
                                if (moveTypeIntoAstNode(context, field->field_value, type->types[idx])) {
                                    propagateTypes(context, field->field_value);
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
                        Type* type = createTypeStruct(&context->types, node, false, names, types, n->count);
                        if (moveTypeIntoAstNode(context, node, type)) {
                            propagateTypes(context, node->parent);
                        }
                    }
                }
                break;
            }
            case AST_TUPLE_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL) {
                    TypeTuple* type = (TypeTuple*)getTypeOfKind(n->res_type, TYPE_TUPLE);
                    if (type != NULL) {
                        for (size_t i = 0; i < n->count && i < type->count; i++) {
                            if (moveTypeIntoAstNode(context, n->nodes[i], type->types[i])) {
                                propagateTypes(context, n->nodes[i]);
                            }
                        }
                    }
                }
                bool has_all = true;
                for (size_t i = 0; i < n->count; i++) {
                    if (n->nodes[i]->res_type == NULL) {
                        has_all = false;
                        break;
                    }
                }
                if (has_all) {
                    Type** types = ALLOC(Type*, n->count);
                    for (size_t i = 0; i < n->count; i++) {
                        types[i] = n->nodes[i]->res_type;
                    }
                    Type* type = createTypeTuple(&context->types, node, types, n->count);
                    if (moveTypeIntoAstNode(context, node, type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            }
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL) {
                    TypeArray* type = (TypeArray*)getTypeOfKind(n->res_type, TYPE_ARRAY);
                    if (type != NULL) {
                        for (size_t i = 0; i < n->count; i++) {
                            if (moveTypeIntoAstNode(context, n->nodes[i], type->base)) {
                                propagateTypes(context, n->nodes[i]);
                            }
                        }
                    }
                }
                Type* type = NULL;
                for (size_t i = 0; i < n->count; i++) {
                    if (n->nodes[i]->res_type != NULL) {
                        type = n->nodes[i]->res_type;
                        break;
                    }
                }
                if (type != NULL) {
                    for (size_t i = 0; i < n->count; i++) {
                        if (moveTypeIntoAstNode(context, n->nodes[i], type)) {
                            propagateTypes(context, n->nodes[i]);
                        }
                    }
                    Type* arr_type = createArrayType(&context->types, node, type, n->count);
                    if (moveTypeIntoAstNode(context, node, arr_type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                if (n->function->name->res_type != NULL) {
                    TypeFunction* func = (TypeFunction*)getTypeOfKind(n->function->name->res_type, TYPE_FUNCTION);
                    if (func != NULL) {
                        if (n->value != NULL) {
                            if (moveTypeIntoAstNode(context, n->value, func->ret_type)) {
                                propagateTypes(context, n->value);
                            }
                        }
                    }
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                if (n->strct->res_type != NULL) {
                    TypeStruct* type = (TypeStruct*)getTypeOfKind(n->strct->res_type, TYPE_STRUCT);
                    if (type != NULL) {
                        size_t idx = lookupIndexOfStructField(type, n->field->name);
                        if (idx != NO_POS) {
                            if (moveTypeIntoAstNode(context, node, type->types[idx])) {
                                propagateTypes(context, node->parent);
                            }
                        }
                    }
                }
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                if (n->tuple->res_type != NULL) {
                    TypeTuple* type = (TypeTuple*)getTypeOfKind(n->tuple->res_type, TYPE_TUPLE);
                    if (type != NULL) {
                        if (n->field->number < type->count) {
                            if (moveTypeIntoAstNode(context, node, type->types[n->field->number])) {
                                propagateTypes(context, node->parent);
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
            case AST_BLOCK_EXPR:
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
                        type->type = getErrorType(&context->types);
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
        FOR_ALL_VAR_REFS(var, {
            moveTypeIntoAstNode(context, (AstNode*)ref, node->res_type);
        })
        moveTypeIntoAstNode(context, (AstNode*)var->def, node->res_type);
    }
}

static void evaluateTypeHints(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
            case AST_FN_TYPE:
            case AST_ARRAY: {
                UNREACHABLE("should not evaluate");
            }
            case AST_ERROR: {
                moveTypeIntoAstNode(context, node, getErrorType(&context->types));
                break;
            }
            case AST_VAR:
            case AST_INT:
            case AST_CHAR:
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
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                evaluateTypeHints(context, n->right);
                evaluateTypeHints(context, n->left);
                break;
            }
            case AST_BOOL: {
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_BOOL));
                break;
            }
            case AST_VOID: {
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                break;
            }
            case AST_STR: {
                moveTypeIntoAstNode(context, node, createPointerType(&context->types, node,
                    createSizedPrimitiveType(&context->types, node, TYPE_UINT, 8)
                ));
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                n->right->res_type = evaluateTypeExpr(context, n->right);
                if (!isValidType(n->right->res_type)) {
                    String type_name = buildTypeName(n->right->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, cast to invalid type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->right->location)
                    ));
                    freeString(type_name);
                    n->right->res_type = getErrorType(&context->types);
                }
                moveTypeIntoAstNode(context, node, n->right->res_type);
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
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                evaluateTypeHints(context, n->tuple);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                Type* bool_type = createUnsizedPrimitiveType(&context->types, node, TYPE_BOOL);
                moveTypeIntoAstNode(context, node, bool_type);
                moveTypeIntoAstNode(context, n->left, bool_type);
                moveTypeIntoAstNode(context, n->right, bool_type);
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
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_BOOL));
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
                if (!isValidType(n->op->res_type)) {
                    String type_name = buildTypeName(n->op->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, sizeof for invalid type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->op->location)
                    ));
                    freeString(type_name);
                    n->op->res_type = getErrorType(&context->types);
                } else if (!isSizedType(n->op->res_type)) {
                    String type_name = buildTypeName(n->op->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_UNSIZED_TYPE,
                        createFormattedString("type error, sizeof for unsized type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is unsized"), n->op->location)
                    ));
                    freeString(type_name);
                    n->op->res_type = getErrorType(&context->types);
                }
                break;
            }
            case AST_CONTINUE:
            case AST_BREAK: {
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
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
            case AST_TUPLE_LIT:
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    evaluateTypeHints(context, n->nodes[i]);
                }
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                for (size_t i = 0; i < n->count; i++) {
                    evaluateTypeHints(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                evaluateTypeHints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK_EXPR: {
                AstBlock* n = (AstBlock*)node;
                evaluateTypeHints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                evaluateTypeHints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                moveTypeIntoAstNode(context, node, createUnsizedPrimitiveType(&context->types, node, TYPE_VOID));
                if (n->type != NULL) {
                    n->name->res_type = evaluateTypeExpr(context, n->type);
                    if (!isValidType(n->name->res_type)) {
                        String type_name = buildTypeName(n->name->res_type);
                        addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                            createFormattedString("type error, variable has invalid type `%s`", cstr(type_name)), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = getErrorType(&context->types);
                    }
                    propagateToVariableReferences(context, n->name);
                }
                evaluateTypeHints(context, n->val);
                break;
            }
            case AST_IF_ELSE_EXPR: {
                AstIfElse* n = (AstIfElse*)node;
                n->condition->res_type = createUnsizedPrimitiveType(&context->types, n->condition, TYPE_BOOL);
                evaluateTypeHints(context, n->condition);
                evaluateTypeHints(context, n->if_block);
                evaluateTypeHints(context, n->else_block);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                n->condition->res_type = createUnsizedPrimitiveType(&context->types, n->condition, TYPE_BOOL);
                evaluateTypeHints(context, n->condition);
                evaluateTypeHints(context, n->if_block);
                evaluateTypeHints(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                n->condition->res_type = createUnsizedPrimitiveType(&context->types, n->condition, TYPE_BOOL);
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
                n->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                evaluateTypeHints(context, (AstNode*)n->arguments);
                Type* ret_type;
                if (n->ret_type != NULL) {
                    ret_type = evaluateTypeExpr(context, n->ret_type);
                } else {
                    ret_type = createUnsizedPrimitiveType(&context->types, (AstNode*)n->name, TYPE_VOID);
                }
                Type** arg_types = ALLOC(Type*, n->arguments->count);
                for (size_t i = 0; i < n->arguments->count; i++) {
                    AstArgDef* def = (AstArgDef*)n->arguments->nodes[i];
                    arg_types[i] = def->name->res_type;
                }
                n->name->res_type = createFunctionType(
                    &context->types, (AstNode*)n->name, ret_type, n->arguments->count, arg_types,
                    (n->flags & AST_FN_FLAG_VARARG) != 0
                );
                if (!isValidType(ret_type)) {
                    String type_name = buildTypeName(ret_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, invalid function return type `%s`", cstr(type_name)), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->ret_type->location)
                    ));
                    freeString(type_name);
                    n->name->res_type = getErrorType(&context->types);
                }
                propagateToVariableReferences(context, n->name);
                evaluateTypeHints(context, n->body);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                n->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                if (n->type != NULL) {
                    n->name->res_type = evaluateTypeExpr(context, n->type);
                    if (!isValidType(n->name->res_type)) {
                        String type_name = buildTypeName(n->name->res_type);
                        addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                            createFormattedString("type error, variable has invalid type `%s`", cstr(type_name)), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("this type is invalid"), n->type->location)
                        ));
                        freeString(type_name);
                        n->name->res_type = getErrorType(&context->types);
                    }
                    propagateToVariableReferences(context, n->name);
                }
                break;
            }
        }
    }
}

static void propagateVariableReferences(CompilerContext* context, AstVar* node) {
    SymbolVariable* var = (SymbolVariable*)node->binding;
    if (var != NULL) {
        FOR_ALL_VAR_REFS(var, {
            propagateTypes(context, ref->parent);
        })
        propagateTypes(context, var->def->parent);
    }
}

static void propagateAllTypes(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
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
                propagateAllTypes(context, n->right);
                propagateAllTypes(context, n->left);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                propagateAllTypes(context, n->left);
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
                propagateAllTypes(context, n->left);
                propagateAllTypes(context, n->right);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                propagateAllTypes(context, n->strct);
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                propagateAllTypes(context, n->tuple);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                propagateAllTypes(context, n->op);
                break;
            }
            case AST_SIZEOF:
            case AST_BREAK:
            case AST_CONTINUE:
                break;
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                propagateAllTypes(context, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    propagateAllTypes(context, field->field_value);
                }
                break;
            }
            case AST_TUPLE_LIT:
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    propagateAllTypes(context, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                propagateAllTypes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                propagateAllTypes(context, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE_EXPR:
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                propagateAllTypes(context, n->condition);
                propagateAllTypes(context, n->if_block);
                propagateAllTypes(context, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                propagateAllTypes(context, n->condition);
                propagateAllTypes(context, n->block);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                propagateAllTypes(context, n->function);
                propagateAllTypes(context, (AstNode*)n->arguments);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                propagateVariableReferences(context, n->name);
                propagateAllTypes(context, (AstNode*)n->arguments);
                propagateAllTypes(context, n->body);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                propagateVariableReferences(context, n->name);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                propagateVariableReferences(context, n->name);
                propagateAllTypes(context, n->val);
                break;
            }
        }
        propagateTypes(context, node);
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

static void assumeAmbiguousTypes(CompilerContext* context, AssumeAmbiguousPhase phase, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
            case AST_ARRAY:
            case AST_FN_TYPE:
                UNREACHABLE("should not evaluate");
            case AST_ERROR:
            case AST_VAR:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_STR:
            case AST_VOID:
            case AST_BOOL:
            case AST_BREAK:
            case AST_CONTINUE:
                break;
            case AST_CHAR:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, node, TYPE_UINT, 8);
                    type = createUnsureType(&context->types, node, UNSURE_INTEGER, type);
                    if (moveTypeIntoAstNode(context, node, type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            case AST_INT:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, node, TYPE_INT, 64);
                    type = createUnsureType(&context->types, node, UNSURE_INTEGER, type);
                    if (moveTypeIntoAstNode(context, node, type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            case AST_REAL:
                if ((phase & ASSUME_LITERALS) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, node, TYPE_REAL, 64);
                    type = createUnsureType(&context->types, node, UNSURE_REAL, type);
                    if (moveTypeIntoAstNode(context, node, type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            case AST_SIZEOF:
                if ((phase & ASSUME_SIZEOF) != 0 && node->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, node, TYPE_UINT, SIZE_SIZE);
                    type = createUnsureType(&context->types, node, UNSURE_INTEGER, type);
                    if (moveTypeIntoAstNode(context, node, type)) {
                        propagateTypes(context, node->parent);
                    }
                }
                break;
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, phase, n->left);
                if ((phase & ASSUME_CASTS) != 0 && n->left->res_type == NULL && n->right->res_type != NULL) {
                    Type* type = n->right->res_type;
                    type = createUnsureType(&context->types, node, UNSURE_ANY, type);
                    if (moveTypeIntoAstNode(context, n->left, type)) {
                        propagateTypes(context, n->left);
                    }
                }
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                assumeAmbiguousTypes(context, phase, n->left);
                assumeAmbiguousTypes(context, phase, n->right);
                if ((phase & ASSUME_INDEX) != 0 && n->right->res_type == NULL) {
                    Type* type = createSizedPrimitiveType(&context->types, n->right, TYPE_UINT, SIZE_SIZE);
                    type = createUnsureType(&context->types, node, UNSURE_INTEGER, type);
                    if (moveTypeIntoAstNode(context, n->right, type)) {
                        propagateTypes(context, n->right);
                    }
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                assumeAmbiguousTypes(context, phase, n->val);
                if ((phase & ASSUME_VARS) != 0 && n->name->res_type == NULL) {
                    Type* type = createUnsureType(&context->types, node, UNSURE_ANY, NULL);
                    if (moveTypeIntoAstNode(context, (AstNode*)n->name, type)) {
                        propagateTypes(context, (AstNode*)n->name);
                    }
                    if (n->val != NULL) {
                        if (moveTypeIntoAstNode(context, n->val, type)) {
                            propagateTypes(context, n->val);
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
                assumeAmbiguousTypes(context, phase, n->right);
                assumeAmbiguousTypes(context, phase, n->left);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                assumeAmbiguousTypes(context, phase, n->strct);
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                assumeAmbiguousTypes(context, phase, n->tuple);
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
                assumeAmbiguousTypes(context, phase, n->left);
                assumeAmbiguousTypes(context, phase, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                assumeAmbiguousTypes(context, phase, n->op);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                assumeAmbiguousTypes(context, phase, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    assumeAmbiguousTypes(context, phase, field->field_value);
                }
                break;
            }
            case AST_TUPLE_LIT:
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    assumeAmbiguousTypes(context, phase, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE_EXPR:
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                assumeAmbiguousTypes(context, phase, n->condition);
                assumeAmbiguousTypes(context, phase, n->if_block);
                assumeAmbiguousTypes(context, phase, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                assumeAmbiguousTypes(context, phase, n->condition);
                assumeAmbiguousTypes(context, phase, n->block);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                assumeAmbiguousTypes(context, phase, (AstNode*)n->arguments);
                assumeAmbiguousTypes(context, phase, n->body);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                assumeAmbiguousTypes(context, phase, n->function);
                assumeAmbiguousTypes(context, phase, (AstNode*)n->arguments);
                break;
            }
        }
    }
}

void typeInferExpr(CompilerContext* context, AstNode* node, Type* assumption) {
    evaluateTypeHints(context, node);
    propagateAllTypes(context, node); 
    assumeAmbiguousTypes(context, ASSUME_LITERALS, node);
    assumeAmbiguousTypes(context, ASSUME_CASTS, node);
    if (assumption != NULL && node->res_type == NULL) {
        Type* type = createUnsureType(&context->types, node, UNSURE_ANY, assumption);
        if (moveTypeIntoAstNode(context, node, type)) {
            propagateTypes(context, node->parent);
        }
    }
}

void runTypeInference(CompilerContext* context) {
    FOR_ALL_MODULES({ evaluateTypeDefinitions(context, file->ast); });
    FOR_ALL_MODULES({ checkTypeDefinitions(context, file->ast); });
    FOR_ALL_MODULES({ evaluateTypeHints(context, file->ast); });
    FOR_ALL_MODULES({ propagateAllTypes(context, file->ast); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_SIZEOF | ASSUME_INDEX, file->ast); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_LITERALS, file->ast); });
    FOR_ALL_MODULES({ assumeAmbiguousTypes(context, ASSUME_CASTS, file->ast); });
    FOR_ALL_MODULES_IF_OK({ checkTypeConstraints(context, file->ast); });
    FOR_ALL_MODULES_IF_OK({ assumeAmbiguousTypes(context, ASSUME_VARS, file->ast); });
}

