
#include "types/eval.h"
#include "ast/astprinter.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "types/check.h"
#include "types/infer.h"

#include "consteval/eval.h"

ConstValue createConstError(CompilerContext* context) {
    ConstValue ret = { .type = getErrorType(&context->types) };
    return ret;
}

static intmax_t wrapSignedInteger(intmax_t value, size_t size) {
    if (size == SIZE_SIZE || size >= 8 * sizeof(value)) {
        return value;
    } else {
        return (value << (8 * sizeof(value) - size)) >> (8 * sizeof(value) - size);
    }
}

ConstValue createConstInt(CompilerContext* context, size_t size, intmax_t value) {
    ConstValue ret = {
        .type = createSizedPrimitiveType(&context->types, NULL, TYPE_INT, size),
        .sint = wrapSignedInteger(value, size),
    };
    return ret;
}

static uintmax_t wrapUnsignedInteger(uintmax_t value, size_t size) {
    if (size == SIZE_SIZE || size >= 8 * sizeof(value)) {
        return value;
    } else {
        return value & ((1 << size) - 1);
    }
}

ConstValue createConstUInt(CompilerContext* context, size_t size, uintmax_t value) {
    ConstValue ret = {
        .type = createSizedPrimitiveType(&context->types, NULL, TYPE_UINT, size),
        .uint = wrapUnsignedInteger(value, size),
    };
    return ret;
}

ConstValue createConstF32(CompilerContext* context, float value) {
    ConstValue ret = { .type = createSizedPrimitiveType(&context->types, NULL, TYPE_REAL, 32), .f32 = value };
    return ret;
}

ConstValue createConstF64(CompilerContext* context, double value) {
    ConstValue ret = { .type = createSizedPrimitiveType(&context->types, NULL, TYPE_REAL, 64), .f64 = value };
    return ret;
}

ConstValue createConstBool(CompilerContext* context, bool value) {
    ConstValue ret = { .type = createUnsizedPrimitiveType(&context->types, NULL, TYPE_BOOL), .boolean = value };
    return ret;
}

static ConstValue raiseOpErrorNotInConst(CompilerContext* context, AstNode* node) {
    const char* ast_name = getAstPrintName(node->kind);
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_NOT_CONSTEXPR,
            createFormattedString("%s expression not allowed in constant expressions", ast_name), 1,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("%s not allowed here", ast_name), node->location)
        )
    );
    return createConstError(context);
}

static ConstValue raiseTypeErrorDifferent(
    CompilerContext* context, AstNode* node, AstNode* left, AstNode* right, Type* type_left, Type* type_right
) {
    String lhs_type = buildTypeName(type_left);
    String rhs_type = buildTypeName(type_right);
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_INCOMPATIBLE_TYPE,
            createFormattedString("type error, incompatible types in %s expression, `%s` and `%s`", getAstPrintName(node->kind), cstr(lhs_type), cstr(rhs_type)), 3,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("types `%s` and `%s` are incompatible", cstr(lhs_type), cstr(rhs_type)), node->location),
            createMessageFragment(MESSAGE_NOTE, createFormattedString("note: lhs has type `%s`", cstr(lhs_type)), left->location),
            createMessageFragment(MESSAGE_NOTE, createFormattedString("note: rhs has type `%s`", cstr(rhs_type)), right->location)
        )
    );
    freeString(lhs_type);
    freeString(rhs_type);
    return createConstError(context);
}

static ConstValue raiseTypeErrorNotInConst(CompilerContext* context, AstNode* node, Type* t) {
    String type = buildTypeName(t);
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_INCOMPATIBLE_TYPE,
            createFormattedString("type error, in constant expession, incompatible type `%s` for %s expession", cstr(type), getAstPrintName(node->kind)), 1,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("`%s` type not allowed here", cstr(type)), node->location)
        )
    );
    freeString(type);
    return createConstError(context);
}

#define BACTION_INTS(ACTION)                    \
    if (isSignedIntegerType(left.type)) {       \
        intmax_t l = left.sint;                 \
        intmax_t r = right.sint;                \
        res = ACTION;                           \
    }

#define BACTION_UINTS(ACTION)                   \
    if (isUnsignedIntegerType(left.type)) {     \
        uintmax_t l = left.uint;                \
        uintmax_t r = right.uint;               \
        res = ACTION;                           \
    }

#define BACTION_FLOATS(ACTION)                  \
    if (isFloatType(left.type)) {               \
        float l = left.f32;                     \
        float r = right.f32;                    \
        res = ACTION;                           \
    }

#define BACTION_DOUBLES(ACTION)                 \
    if (isDoubleType(left.type)) {              \
        double l = left.f64;                    \
        double r = right.f64;                   \
        res = ACTION;                           \
    }

#define BACTION_BOOLS(ACTION)                   \
    if (isBooleanType(left.type)) {             \
        bool l = left.boolean;                  \
        bool r = right.boolean;                 \
        res = ACTION;                           \
    }

#define BACTIONS_T(ACTIONS)                                             \
    TypeSizedPrimitive* t = (TypeSizedPrimitive*)left.type;             \
    BACTIONS(ACTIONS)

#define BACTIONS(ACTIONS)                                               \
    ACTIONS else {                                                      \
        res = raiseTypeErrorNotInConst(context, n->left, left.type);    \
    }

#define BACTION_ANY_INTS(ACTION) \
    BACTION_INTS(createConstInt(context, t->size, ACTION)) else BACTION_UINTS(createConstUInt(context, t->size, ACTION))

#define BACTION_INT(ACTION) \
    BACTIONS_T(BACTION_ANY_INTS(ACTION))

#define BACTION_ANY_FLOATS(ACTION) \
    BACTION_FLOATS(createConstF32(context, ACTION)) else BACTION_DOUBLES(createConstF64(context, ACTION))

#define BACTION_NUM(ACTION) \
    BACTIONS_T(BACTION_ANY_INTS(ACTION) else BACTION_ANY_FLOATS(ACTION))

#define BACTION_CMP(ACTION) BACTIONS(                                                                               \
    BACTION_INTS(createConstBool(context, ACTION)) else BACTION_UINTS(createConstBool(context, ACTION))             \
    else BACTION_FLOATS(createConstBool(context, ACTION)) else BACTION_DOUBLES(createConstBool(context, ACTION))    \
    else BACTION_BOOLS(createConstBool(context, ACTION))                                                            \
)

#define BACTION_BOOL(ACTION)                                            \
    BACTION_BOOLS(createConstBool(context, ACTION)) else {              \
        res = raiseTypeErrorNotInConst(context, n->left, left.type);    \
    }

#define BINARY_OP(ACTION) {                                                                         \
    AstBinary* n = (AstBinary*)node;                                                                \
    ConstValue left = evaluateConstExpr(context, n->left);                                          \
    if (isErrorType(left.type)) {                                                                   \
        res = left;                                                                                 \
    } else {                                                                                        \
        ConstValue right = evaluateConstExpr(context, n->right);                                    \
        if (isErrorType(right.type)) {                                                              \
            res = right;                                                                            \
        } else if (!compareStructuralTypes(left.type, right.type)) {                                \
            res = raiseTypeErrorDifferent(context, node, n->left, n->right, left.type, right.type); \
        } else { ACTION }                                                                           \
    }                                                                                               \
    break;                                                                                          \
}

#define UACTION_INTS(ACTION)                                \
    if (isSignedIntegerType(op.type)) {                     \
        intmax_t o = op.sint;                               \
        res = createConstInt(context, t->size, ACTION);     \
    } else if (isUnsignedIntegerType(op.type)) {            \
        uintmax_t o = op.uint;                              \
        res = createConstUInt(context, t->size, ACTION);    \
    }

#define UACTION_FLOATS(ACTION)                              \
    if (isFloatType(op.type)) {                             \
        float o = op.f32;                                   \
        res = createConstF32(context, ACTION);              \
    } else if (isDoubleType(op.type)) {                     \
        double o = op.f64;                                  \
        res = createConstF64(context, ACTION);              \
    }

#define UACTION_BOOLS(ACTION)                       \
    if (isBooleanType(op.type)) {                   \
        bool o = op.boolean;                        \
        res = createConstBool(context, ACTION);     \
    }

#define UACTIONS_T(ACTIONS)                                             \
    TypeSizedPrimitive* t = (TypeSizedPrimitive*)op.type;               \
    UACTIONS(ACTIONS)

#define UACTIONS(ACTIONS)                                               \
    ACTIONS else {                                                      \
        res = raiseTypeErrorNotInConst(context, n->op, op.type);        \
    }

#define UACTION_NUM(ACTION) UACTIONS_T(UACTION_INTS(ACTION) else UACTION_FLOATS(ACTION))

#define UNARY_OP(ACTION) {                              \
    AstUnary* n = (AstUnary*)node;                      \
    ConstValue op = evaluateConstExpr(context, n->op);  \
    if (isErrorType(op.type)) {                         \
        res = op;                                       \
    } else { ACTION }                                   \
    break;                                              \
}

#define RECURSION_CHECK_TYPE ((Type*)-1)

ConstValue evaluateConstExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE("should not evaluate");
    } else {
        ConstValue res = { .type = NULL };
        switch (node->kind) {
            case AST_ERROR: {
                res = createConstError(context);
                break;
            }
            case AST_IF_ELSE:
            case AST_FN:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_WHILE:
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
            case AST_FN_TYPE:
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
            case AST_ASSIGN:
            case AST_RETURN:
            case AST_BREAK:
            case AST_CONTINUE:
            case AST_ROOT:
            case AST_ARRAY:
            case AST_LIST:
            case AST_BLOCK:
            case AST_STATICDEF:
            case AST_CONSTDEF:
            case AST_VARDEF:
            case AST_INDEX:
            case AST_VOID:
            case AST_ARRAY_LIT:
            case AST_TUPLE_LIT:
            case AST_STRUCT_INDEX:
            case AST_TUPLE_INDEX:
            case AST_STRUCT_LIT:
            case AST_CALL:
            case AST_STR:
            case AST_SIZEOF:
            case AST_ADDR:
            case AST_DEREF:
                UNREACHABLE("should not evaluate");
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var->value.type == RECURSION_CHECK_TYPE) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_CONST,
                        createFormattedString("recursive reference in constant definition of `%s`", var->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("recursive reference to constant"), node->location)
                    ));
                    res = createConstError(context);
                } else {
                    if (var->value.type == NULL) {
                        evaluateConstantDefinition(context, (AstVarDef*)var->def->parent);
                    }
                    res = var->value;
                }
                break;
            }
            case AST_BLOCK_EXPR: {
                AstBlock* n = (AstBlock*)node;
                for (size_t i = 0; i < n->nodes->count; i++) {
                    res = evaluateConstExpr(context, n->nodes->nodes[i]);
                }
                break;
            }
            case AST_IF_ELSE_EXPR: {
                AstIfElse* n = (AstIfElse*)node;
                ConstValue cond = evaluateConstExpr(context, n->condition);
                if (isBooleanType(cond.type)) {
                    if (cond.boolean) {
                        res = evaluateConstExpr(context, n->if_block);
                    } else {
                        res = evaluateConstExpr(context, n->else_block);
                    }
                } else {
                    res = raiseTypeErrorNotInConst(context, n->condition, n->condition->res_type);
                }
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                if (n->res_type == NULL) {
                    n->res_type = evaluateTypeExpr(context, n->right);
                }
                ConstValue op = evaluateConstExpr(context, n->left);
                size_t size = getIntRealTypeSize(n->res_type);
                if (isRealType(n->res_type)) {
                    size_t op_size = getIntRealTypeSize(n->left->res_type);
                    if (isRealType(n->left->res_type)) {
                        if (size == op_size) {
                            res = op;
                            break;
                        } else if (size == 32) {
                            res = createConstF32(context, (float)op.f64);
                            break;
                        } else if (size == 64) {
                            res = createConstF64(context, (double)op.f32);
                            break;
                        }
                    } else if (isSignedIntegerType(n->left->res_type)) {
                        if (size == 32) {
                            res = createConstF32(context, (float)op.sint);
                            break;
                        } else if (size == 64) {
                            res = createConstF64(context, (double)op.sint);
                            break;
                        }
                    } else if (isUnsignedIntegerType(n->left->res_type)) {
                        if (size == 32) {
                            res = createConstF32(context, (float)op.uint);
                            break;
                        } else if (size == 64) {
                            res = createConstF64(context, (double)op.uint);
                            break;
                        }
                    }
                } else if (isSignedIntegerType(n->res_type)) {
                    if (isFloatType(n->left->res_type)) {
                        res = createConstInt(context, size, (intmax_t)op.f32);
                        break;
                    } else if (isDoubleType(n->left->res_type)) {
                        res = createConstInt(context, size, (intmax_t)op.f64);
                        break;
                    } else if (isSignedIntegerType(n->left->res_type)) {
                        res = createConstInt(context, size, (intmax_t)op.sint);
                        break;
                    } else if (isUnsignedIntegerType(n->left->res_type)) {
                        res = createConstInt(context, size, (intmax_t)op.uint);
                        break;
                    }
                } else if (isUnsignedIntegerType(n->res_type)) {
                    if (isFloatType(n->left->res_type)) {
                        res = createConstUInt(context, size, (uintmax_t)op.f32);
                        break;
                    } else if (isDoubleType(n->left->res_type)) {
                        res = createConstUInt(context, size, (uintmax_t)op.f64);
                        break;
                    } else if (isSignedIntegerType(n->left->res_type)) {
                        res = createConstUInt(context, size, (uintmax_t)op.sint);
                        break;
                    } else if (isUnsignedIntegerType(n->left->res_type)) {
                        res = createConstUInt(context, size, (uintmax_t)op.uint);
                        break;
                    }
                }
                res = raiseTypeErrorNotInConst(context, n->left, n->left->res_type);
                break;
            }
            case AST_ADD: BINARY_OP(BACTION_NUM(l + r))
            case AST_SUB: BINARY_OP(BACTION_NUM(l - r))
            case AST_MUL: BINARY_OP(BACTION_NUM(l * r))
            case AST_DIV: BINARY_OP(BACTION_NUM(l / r))
            case AST_MOD: BINARY_OP(BACTION_INT(l % r))
            case AST_SHL: BINARY_OP(BACTION_INT(l << r))
            case AST_SHR: BINARY_OP(BACTION_INT(l >> r))
            case AST_BAND: BINARY_OP(BACTION_INT(l & r))
            case AST_BOR: BINARY_OP(BACTION_INT(l | r))
            case AST_BXOR: BINARY_OP(BACTION_INT(l ^ r))
            case AST_EQ: BINARY_OP(BACTION_CMP(l == r))
            case AST_NE: BINARY_OP(BACTION_CMP(l != r))
            case AST_LE: BINARY_OP(BACTION_CMP(l <= r))
            case AST_GE: BINARY_OP(BACTION_CMP(l >= r))
            case AST_LT: BINARY_OP(BACTION_CMP(l < r))
            case AST_GT: BINARY_OP(BACTION_CMP(l > r))
            case AST_OR: BINARY_OP(BACTION_BOOL(l || r))
            case AST_AND: BINARY_OP(BACTION_BOOL(l && r))
            case AST_POS: UNARY_OP(UACTION_NUM(o))
            case AST_NEG: UNARY_OP(UACTION_NUM(-o))
            case AST_NOT: UNARY_OP(UACTIONS_T(UACTION_INTS(~o) else UACTION_BOOLS(!o)))
            case AST_CHAR:
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                if (!isErrorType(n->res_type)) {
                    if (n->res_type == NULL) {
                        n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_INT, 64);
                    }
                    if (isSignedIntegerType(n->res_type)) {
                        res = createConstInt(context, getIntRealTypeSize(n->res_type), n->number);
                    } else if (isUnsignedIntegerType(n->res_type)) {
                        res = createConstUInt(context, getIntRealTypeSize(n->res_type), n->number);
                    } else {
                        UNREACHABLE("integer type expected");
                    }
                } else {
                    res = createConstError(context);
                }
                break;
            }
            case AST_REAL: {
                AstReal* n = (AstReal*)node;
                if (!isErrorType(n->res_type)) {
                    if (n->res_type == NULL) {
                        n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_REAL, 64);
                    }
                    if (isFloatType(n->res_type)) {
                        res = createConstF32(context, n->number);
                    } else if (isDoubleType(n->res_type)) {
                        res = createConstF64(context, n->number);
                    } else {
                        UNREACHABLE("unexpected real type size");
                    }
                } else {
                    res = createConstError(context);
                }
                break;
            }
            case AST_BOOL: {
                AstBool* n = (AstBool*)node;
                if (!isErrorType(n->res_type)) {
                    if (n->res_type == NULL) {
                        n->res_type = createUnsizedPrimitiveType(&context->types, NULL, TYPE_BOOL);
                    }
                    if (isBooleanType(n->res_type)) {
                        res = createConstBool(context, n->value);
                    } else {
                        UNREACHABLE("boolean type expected");
                    }
                } else {
                    res = createConstError(context);
                }
                break;
            }
        }
        node->res_type = res.type;
        return res;
    }
}

bool checkValidInConstExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE("should not evaluate");
    } else {
        switch (node->kind) {
            case AST_ERROR:
                return true;
            case AST_IF_ELSE:
            case AST_FN:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_WHILE:
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
            case AST_FN_TYPE:
            case AST_STRUCT_TYPE:
            case AST_TUPLE_TYPE:
            case AST_ASSIGN:
            case AST_RETURN:
            case AST_BREAK:
            case AST_CONTINUE:
            case AST_ROOT:
            case AST_ARRAY:
            case AST_LIST:
            case AST_BLOCK:
            case AST_STATICDEF:
            case AST_CONSTDEF:
            case AST_VARDEF:
            case AST_VOID:
            case AST_INDEX: // TODO: constant arrays?
            case AST_ARRAY_LIT:
            case AST_TUPLE_INDEX: // TODO: tuple in const?
            case AST_TUPLE_LIT:
            case AST_STRUCT_INDEX: // TODO: structs in const?
            case AST_STRUCT_LIT:
            case AST_CALL: // TODO: constant calls?
            case AST_STR: // TODO: constant strings?
            case AST_SIZEOF: // TODO: would need llvm to make this const?
            case AST_ADDR:
            case AST_DEREF: {
                // None of these are allowed in constant expressions.
                raiseOpErrorNotInConst(context, node);
                return false;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var != NULL && var->constant) {
                    return true;
                } else {
                    raiseOpErrorNotInConst(context, node);
                    return false;
                }
            }
            case AST_BLOCK_EXPR: {
                AstBlock* n = (AstBlock*)node;
                for (size_t i = 0; i < n->nodes->count; i++) {
                    if (!checkValidInConstExpr(context, n->nodes->nodes[i])) {
                        return false;
                    }
                }
                return true;
            }
            case AST_IF_ELSE_EXPR: {
                AstIfElse* n = (AstIfElse*)node;
                return checkValidInConstExpr(context, n->condition)
                    && checkValidInConstExpr(context, n->if_block)
                    && checkValidInConstExpr(context, n->else_block);
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                return checkValidInConstExpr(context, n->left);
            }
            case AST_ADD:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
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
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                return checkValidInConstExpr(context, n->left)
                       && checkValidInConstExpr(context, n->right);
            }
            case AST_POS:
            case AST_NEG:
            case AST_NOT: {
                AstUnary* n = (AstUnary*)node;
                return checkValidInConstExpr(context, n->op);
            }
            case AST_CHAR:
            case AST_INT:
            case AST_REAL:
            case AST_BOOL: {
                return true;
            }
        }
        return false;
    }
}

void evaluateConstantDefinition(CompilerContext* context, AstVarDef* def) {
    size_t old_error = context->msgs.error_count;
    if (checkValidInConstExpr(context, def->val)) {
        typeInferExpr(context, (AstNode*)def, NULL);
        typeCheckExpr(context, (AstNode*)def);
    }
    if (context->msgs.error_count == old_error && def->name->binding != NULL) {
        SymbolVariable* var = (SymbolVariable*)def->name->binding;
        var->value.type = RECURSION_CHECK_TYPE;
        var->value = evaluateConstExpr(context, def->val);
    }
}

