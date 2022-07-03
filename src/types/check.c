
#include "ast/astprinter.h"
#include "types/eval.h"
#include "errors/fatalerror.h"

#include "types/check.h"

static void checkForUntypedVariables(CompilerContext* context, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_TUPLE_TYPE:
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
            case AST_BREAK:
            case AST_CONTINUE:
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
            case AST_TUPLE_LIT:
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
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                checkForUntypedVariables(context, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE_EXPR:
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
                        createFormattedString("type error, unable to infer the type of parameter `%s`", n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("unable to infer the type of this parameter"), n->name->location)
                    ));
                }
                break;
            }
            case AST_CONSTDEF:
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                if (isPartialType(n->name->res_type)) {
                    const char* type = node->kind == AST_CONSTDEF ? "constant" : "variable";
                    addMessageToContext(
                        &context->msgs,
                        createMessage(
                            ERROR_UNINFERRED_TYPE,
                            createFormattedString(
                                "type error, unable to infer the type of %s `%s`", type,
                                n->name->name
                            ),
                            1,
                            createMessageFragment(
                                MESSAGE_ERROR,
                                createFormattedString("unable to infer the type of this %s", type),
                                n->name->location
                            )
                        )
                    );
                    fillPartialType(n->name->res_type, getErrorType(&context->types));
                }
                checkForUntypedVariables(context, n->val);
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                checkForUntypedVariables(context, n->strct);
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                checkForUntypedVariables(context, n->tuple);
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
                case AST_TUPLE_TYPE:
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
                case AST_BREAK:
                case AST_CONTINUE:
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
                case AST_TUPLE_LIT:
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
                case AST_BLOCK_EXPR:
                case AST_BLOCK: {
                    AstBlock* n = (AstBlock*)node;
                    checkForUntypedNodes(context, (AstNode*)n->nodes);
                    break;
                }
                case AST_IF_ELSE_EXPR:
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
                case AST_CONSTDEF:
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
                case AST_TUPLE_INDEX: {
                    AstTupleIndex* n = (AstTupleIndex*)node;
                    checkForUntypedNodes(context, n->tuple);
                    break;
                }
            }
        }
    }
}

static void raiseLiteralTypeError(CompilerContext* context, AstNode* node, const char* kind) {
    String actual_type = buildTypeName(node->res_type);
    String message = createFormattedString(
        "type error, expecting expression of type `%s` but found %s", cstr(actual_type), kind
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("%ss are not of type `%s`", kind, cstr(actual_type)),
        node->location
    );
    if (getTypeReason(node->res_type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    createFormattedString("note: expecting `%s` because of this", cstr(actual_type)),
                    getTypeReason(node->res_type)->location
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
    CompilerContext* context, AstNode* node, AstNode* err_node, Type* type,
    const char* hint, const char* help, Span help_location
) {
    String type_name = buildTypeName(type);
    String message = createFormattedString(
        "type error, incompatible type `%s` for %s expession%s", cstr(type_name), getAstPrintName(node->kind), hint
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("`%s` type not allowed here", cstr(type_name)), err_node->location
    );
    MessageFragment* frags[2];
    size_t frag_count = 0;
    if (getTypeReason(type) != NULL) {
        frags[frag_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: type `%s` defined here", cstr(type_name)), getTypeReason(type)->location
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
    CompilerContext* context, AstNode* node, AstNode* err_node, Type* type, const char* hint
) {
    raiseOpTypeErrorWithHelp(context, node, err_node, type, hint, NULL, invalidSpan());
}

static void raiseArgCountError(CompilerContext* context, TypeFunction* type, AstList* arguments) {
    String message = createFormattedString(
        "too %s arguments to call expression, expected %zi, found %zi",
        type->arg_count < arguments->count ? "many" : "few",
        type->arg_count, arguments->count
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("expected %zi argument%s", type->arg_count, (type->arg_count != 1 ? "s" : "")), arguments->location
    );
    if (getTypeReason((Type*)type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_ARGUMENT_COUNT, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: function defined here"),
                    getTypeReason((Type*)type)->location
                )
            )
        );
    } else {
        addMessageToContext(
            &context->msgs, createMessage(ERROR_ARGUMENT_COUNT, message, 1, error)
        );
    }
}

static void raiseVoidReturnError(CompilerContext* context, AstReturn* node, Type* type) {
    String type_name = buildTypeName(type);
    String message = createFormattedString("type error, expected a return value of type `%s`", cstr(type_name));
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("should return value of type `%s`", cstr(type_name)),
        node->location
    );
    if (getTypeReason(type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    createFormattedString("note: expecting `%s` because of this", cstr(type_name)),
                    getTypeReason(type)->location
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

static void raiseUnsupportedCast(CompilerContext* context, AstNode* node, Type* from, Type* to) {
    String fst_type = buildTypeName(from);
    String snd_type = buildTypeName(to);
    String message =
        createFormattedString("type error, unsupported cast form `%s` to `%s`", cstr(fst_type), cstr(snd_type));
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR,
        createFormattedString(
            "unsupported cast from `%s` to `%s`", cstr(fst_type), cstr(snd_type)
        ),
        node->location
    );
    MessageFragment* notes[2];
    size_t note_count = 0;
    if (getTypeReason(from) != NULL) {
        notes[note_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: expecting `%s` because of this", cstr(fst_type)),
            getTypeReason(from)->location
        );
        note_count++;
    }
    if (getTypeReason(to) != NULL) {
        notes[note_count] = createMessageFragment(
            MESSAGE_NOTE, createFormattedString("note: expecting `%s` because of this", cstr(snd_type)),
            getTypeReason(to)->location
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
    node->res_type = getErrorType(&context->types);
}

static void raiseNoSuchFieldError(CompilerContext* context, AstStructIndex* node) {
    String type_name = buildTypeName(node->strct->res_type);
    String message = createFormattedString(
        "no field with name `%s` in struct type `%s`", node->field->name, cstr(type_name)
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("no such field exists"), node->field->location
    );
    if (getTypeReason(node->strct->res_type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_NO_SUCH_FIELD, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: struct type defined here"),
                    getTypeReason(node->strct->res_type)->location
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

static void raiseNoSuchTupleFieldError(CompilerContext* context, AstTupleIndex* node) {
    String type_name = buildTypeName(node->tuple->res_type);
    String message = createFormattedString(
        "no field at index `%zu` in tuple type `%s`", node->field->number, cstr(type_name)
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("no such field exists"), node->field->location
    );
    if (getTypeReason(node->tuple->res_type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_NO_SUCH_FIELD, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: tuple type defined here"),
                    getTypeReason(node->tuple->res_type)->location
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
        "inconsistent fields in struct literal, expected literal of type `%s`", cstr(type_name)
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, copyFromCString("mismatch in struct fields"), node->location
    );
    if (getTypeReason(node->res_type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_NO_SUCH_FIELD, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    copyFromCString("note: struct type defined here"),
                    getTypeReason(node->res_type)->location
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

void raiseLiteralLengthMismatchError(CompilerContext* context, AstNode* node, const char* kind) {
    String type_name = buildTypeName(node->res_type);
    String message = createFormattedString(
        "inconsistent length of %s literal, expected literal of type `%s`", kind, cstr(type_name)
    );
    MessageFragment* error = createMessageFragment(
        MESSAGE_ERROR, createFormattedString("mismatch in %s lengths", kind), node->location
    );
    if (getTypeReason(node->res_type) != NULL) {
        addMessageToContext(
            &context->msgs,
            createMessage(
                ERROR_INCOMPATIBLE_TYPE, message, 2, error,
                createMessageFragment(
                    MESSAGE_NOTE,
                    createFormattedString("note: %s type defined here", kind),
                    getTypeReason(node->res_type)->location
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
    } else if (node->kind == AST_VAR) {
        AstVar* n = (AstVar*)node;
        SymbolVariable* var = (SymbolVariable*)n->binding;
        return !var->constant;
    } else if (node->kind == AST_DEREF) {
        return true;
    } else if (node->kind == AST_INDEX) {
        AstBinary* n = (AstBinary*)node;
        if (isPointerType(n->left->res_type)) {
            return true;
        } else {
            return isAddressableValue(n->left);
        }
    } else if (node->kind == AST_STRUCT_INDEX) {
        AstStructIndex* n = (AstStructIndex*)node;
        return isAddressableValue(n->strct);
    } else if (node->kind == AST_TUPLE_INDEX) {
        AstTupleIndex* n = (AstTupleIndex*)node;
        return isAddressableValue(n->tuple);
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
    return isAddressableValue(node) && (node->res_type == NULL || isSizedType(node->res_type));
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

void checkTypeConstraints(CompilerContext* context, AstNode* node) {
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
            case AST_BREAK:
            case AST_CONTINUE:
                break;
            case AST_VOID: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isVoidType(node->res_type)) {
                        raiseLiteralTypeError(context, node, "`()`");
                    }
                }
                break;
            }
            case AST_STR: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    TypePointer* ptr_type = (TypePointer*)getTypeOfKind(node->res_type, TYPE_POINTER);
                    if (ptr_type == NULL || !isUnsignedIntegerType(ptr_type->base) || getIntRealTypeSize(ptr_type->base) != 8) {
                        raiseLiteralTypeError(context, node, "string literal");
                    }
                }
                break;
            }
            case AST_CHAR:
            case AST_INT: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isIntegerType(node->res_type)) {
                        raiseLiteralTypeError(context, node, node->kind == AST_INT ? "integer literal" : "character literal");
                    }
                }
                break;
            }
            case AST_BOOL: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isBooleanType(node->res_type)) {
                        raiseLiteralTypeError(context, node, "boolean literal");
                    }
                }
                break;
            }
            case AST_SIZEOF: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isIntegerType(node->res_type)) {
                        raiseLiteralTypeError(context, node, "sizeof expression");
                    }
                }
                break;
            }
            case AST_REAL: {
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isRealType(node->res_type)) {
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
                    if (!isNumericType(n->left->res_type)) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, ", must be a numeric value");
                        break;
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
                    if (!isIntegerType(n->left->res_type)) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, ", must be an integer value");
                        break;
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
                    if (isIntegerType(n->res_type)) {
                        if (!isIntegerType(n->left->res_type) && !isPointerType(n->left->res_type) && !isRealType(n->left->res_type)) {
                            raiseUnsupportedCast(context, node, n->left->res_type, n->res_type);
                            break;
                        }
                    } else if (isRealType(n->res_type)) {
                        if (!isRealType(n->left->res_type) && !isIntegerType(n->left->res_type)) {
                            raiseUnsupportedCast(context, node, n->left->res_type, n->res_type);
                            break;
                        }
                    } else if (isPointerType(n->res_type)) {
                        if (!isPointerType(n->left->res_type) && !isIntegerType(n->left->res_type)) {
                            raiseUnsupportedCast(context, node, n->left->res_type, n->res_type);
                            break;
                        }
                    } else {
                        raiseUnsupportedCast(context, node, n->left->res_type, n->res_type);
                        break;
                    }
                }
                checkTypeConstraints(context, n->left);
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                if (n->left->res_type != NULL && !isErrorType(n->left->res_type)) {
                    if (!isArrayType(n->left->res_type)) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, ", must be an array");
                        break;
                    }
                }
                if (n->right->res_type != NULL && !isErrorType(n->right->res_type) && !isIntegerType(n->right->res_type)) {
                    raiseOpTypeError(context, node, n->right, n->right->res_type, ", must be an integer value");
                    break;
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
                if (n->res_type != NULL && !isErrorType(n->res_type) && !isNumericType(n->res_type)) {
                    raiseOpTypeError(context, node, n->left, n->res_type, ", must be a numeric value");
                    break;
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
                if (n->res_type != NULL && !isErrorType(n->res_type) && !isIntegerType(n->res_type)) {
                    raiseOpTypeError(context, node, n->left, n->res_type, ", must be an integer value");
                    break;
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
                    if (!isNumericType(n->left->res_type) && !isBooleanType(n->left->res_type) && !isPointerType(n->left->res_type)) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, ", must be a numeric value, boolean or pointer");
                        break;
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
                    if (!isNumericType(n->left->res_type) && !isPointerType(n->left->res_type)) {
                        raiseOpTypeError(context, node, n->left, n->left->res_type, ", must be a numeric value or pointer");
                        break;
                    }
                }
                checkTypeConstraints(context, n->left);
                checkTypeConstraints(context, n->right);
                break;
            }
            case AST_POS: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && !isNumericType(n->res_type)) {
                    raiseOpTypeError(context, node, n->op, n->res_type, ", must be a numeric value");
                    break;
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_NEG: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && !isRealType(n->res_type) && !isSignedIntegerType(n->res_type)) {
                    raiseOpTypeError(context, node, n->op, n->res_type, ", must be a signed numeric value");
                    break;
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_NOT: {
                AstUnary* n = (AstUnary*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type) && !isIntegerType(n->res_type) && !isBooleanType(n->res_type)) {
                    raiseOpTypeError(context, node, n->op, n->res_type, ", must be an integer or boolean value");
                    break;
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                if (node->res_type != NULL && !isErrorType(node->res_type)) {
                    if (!isPointerType(node->res_type)) {
                        raiseOpTypeError(context, node, node, node->res_type, ", always returns a pointer");
                        break;
                    }
                }
                checkNodeIsAddressable(context, n->op);
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                if (n->op->res_type != NULL && !isErrorType(n->op->res_type)) {
                    if (!isPointerType(n->op->res_type)) {
                        raiseOpTypeError(context, node, n->op, n->op->res_type, ", must be a pointer");
                        break;
                    }
                }
                checkTypeConstraints(context, n->op);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                if (n->function->name->res_type != NULL && !isErrorType(n->function->name->res_type)) {
                    TypeFunction* func = (TypeFunction*)getTypeOfKind(n->function->name->res_type, TYPE_FUNCTION);
                    ASSERT(func != NULL);
                    if (n->value == NULL && !isVoidType(func->ret_type)) {
                        raiseVoidReturnError(context, n, func->ret_type);
                        break;
                    }
                }
                checkTypeConstraints(context, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type)) {
                    TypeStruct* type = (TypeStruct*)getTypeOfKind(n->res_type, TYPE_STRUCT);
                    if (type != NULL) {
                        sortStructFieldsByName(n);
                        if (type->count != n->count) {
                            raiseStructFieldMismatchError(context, node);
                            break;
                        } else {
                            size_t i = 0;
                            for (; i < type->count; i++) {
                                AstStructField* field = (AstStructField*)n->nodes[i];
                                if (type->names[i] != field->name->name) {
                                    raiseStructFieldMismatchError(context, node);
                                    break;
                                }
                            }
                            if (i < type->count) {
                                break;
                            }
                        }
                    } else {
                        raiseLiteralTypeError(context, node, "struct literal");
                        break;
                    }
                }
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    checkTypeConstraints(context, field->field_value);
                }
                break;
            }
            case AST_TUPLE_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type)) {
                    TypeTuple* type = (TypeTuple*)getTypeOfKind(n->res_type, TYPE_TUPLE);
                    if (type != NULL) {
                        if (type->count != n->count) {
                            raiseLiteralLengthMismatchError(context, node, "tuple");
                            break;
                        }
                    } else {
                        raiseLiteralTypeError(context, node, "tuple literal");
                        break;
                    }
                }
                for (size_t i = 0; i < n->count; i++) {
                    checkTypeConstraints(context, n->nodes[i]);
                }
                break;
            }
            case AST_ARRAY_LIT: {
                AstList* n = (AstList*)node;
                if (n->res_type != NULL && !isErrorType(n->res_type)) {
                    TypeArray* type = (TypeArray*)getTypeOfKind(n->res_type, TYPE_ARRAY);
                    if (type != NULL) {
                        if (type->size != n->count) {
                            raiseLiteralLengthMismatchError(context, node, "array");
                            break;
                        }
                    } else {
                        raiseLiteralTypeError(context, node, "array literal");
                        break;
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
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                checkTypeConstraints(context, (AstNode*)n->nodes);
                break;
            }
            case AST_IF_ELSE_EXPR:
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
                    TypeFunction* type = (TypeFunction*)getTypeOfKind(n->function->res_type, TYPE_FUNCTION);
                    if (type == NULL) {
                        TypePointer* ptr_type = (TypePointer*)getTypeOfKind(n->function->res_type, TYPE_POINTER);
                        if (ptr_type != NULL && isFunctionType(ptr_type->base)) {
                            raiseOpTypeErrorWithHelp(
                                context, node, n->function, n->function->res_type, ", must be a function",
                                "help: consider dereferencing before the function call, e.g. `(*_)(..)`", invalidSpan()
                            );
                            break;
                        } else {
                            raiseOpTypeError(context, node, n->function, n->function->res_type, ", must be a function");
                            break;
                        }
                    } else if (type->arg_count != n->arguments->count && (!type->vararg || type->arg_count > n->arguments->count)) {
                        raiseArgCountError(context, type, n->arguments);
                        break;
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
                        createFormattedString("type error, unsized type `%s` for parameter `%s`", cstr(type_name), n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("parameter with unsized type"), n->name->location)
                    ));
                    freeString(type_name);
                    break;
                }
                break;
            }
            case AST_CONSTDEF:
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                checkTypeConstraints(context, n->val);
                if (n->name->res_type != NULL && !isErrorType(n->name->res_type) && !isSizedType(n->name->res_type)) {
                    const char* type = node->kind == AST_CONSTDEF ? "constant" : "variable";
                    String type_name = buildTypeName(n->name->res_type);
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_TYPE,
                        createFormattedString("type error, unsized type `%s` for %s `%s`", cstr(type_name), type, n->name->name), 1,
                        createMessageFragment(MESSAGE_ERROR, createFormattedString("%s with unsized type", type), n->name->location)
                    ));
                    freeString(type_name);
                    break;
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                checkTypeConstraints(context, n->strct);
                if (n->strct->res_type != NULL && !isErrorType(n->strct->res_type)) {
                    TypeStruct* type = (TypeStruct*)getTypeOfKind(n->strct->res_type, TYPE_STRUCT);
                    if (type == NULL) {
                        raiseOpTypeError(
                            context, node, n->strct, n->strct->res_type, ", must be a struct"
                        );
                        break;
                    } else if (lookupIndexOfStructField(type, n->field->name) == NO_POS) {
                        raiseNoSuchFieldError(context, n);
                        break;
                    }
                }
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                checkTypeConstraints(context, n->tuple);
                if (n->tuple->res_type != NULL && !isErrorType(n->tuple->res_type)) {
                    TypeTuple* type = (TypeTuple*)getTypeOfKind(n->tuple->res_type, TYPE_TUPLE);
                    if (type == NULL) {
                        raiseOpTypeError(
                            context, node, n->tuple, n->tuple->res_type, ", must be a tuple"
                        );
                        break;
                    } else if (n->field->number >= type->count) {
                        raiseNoSuchTupleFieldError(context, n);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void typeCheckExpr(CompilerContext* context, AstNode* node) {
    if (context->msgs.error_count == 0) {
        checkForUntypedVariables(context, node); 
    }
    if (context->msgs.error_count == 0) {
        checkTypeConstraints(context, node);
    }
    if (context->msgs.error_count == 0) {
        checkForUntypedNodes(context, node);
    }
}

void runTypeChecking(CompilerContext* context) {
    FOR_ALL_MODULES_IF_OK({ checkForUntypedVariables(context, file->ast); });
    FOR_ALL_MODULES_IF_OK({ checkTypeConstraints(context, file->ast); });
    FOR_ALL_MODULES_IF_OK({ checkForUntypedNodes(context, file->ast); });
}
