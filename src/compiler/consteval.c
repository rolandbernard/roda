
#include "ast/astprinter.h"
#include "errors/fatalerror.h"
#include "text/format.h"

#include "compiler/consteval.h"

ConstValue createConstError(CompilerContext* context) {
    ConstValue ret = { .type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR) };
    return ret;
}

static intmax_t wrapSignedInteger(intmax_t value, size_t size) {
    return (value << (8 * sizeof(value) - size)) >> (8 * sizeof(value) - size);
}

ConstValue createConstInt(CompilerContext* context, size_t size, intmax_t value) {
    ConstValue ret = {
        .type = createSizedPrimitiveType(&context->types, TYPE_INT, size),
        .sint = wrapSignedInteger(value, size),
    };
    return ret;
}

static uintmax_t wrapUnsignedInteger(uintmax_t value, size_t size) {
    return value & ((1 << size) - 1);
}

ConstValue createConstUInt(CompilerContext* context, size_t size, uintmax_t value) {
    ConstValue ret = {
        .type = createSizedPrimitiveType(&context->types, TYPE_UINT, size),
        .uint = wrapUnsignedInteger(value, size),
    };
    return ret;
}

ConstValue createConstF32(CompilerContext* context, float value) {
    ConstValue ret = { .type = createSizedPrimitiveType(&context->types, TYPE_REAL, 32), .f32 = value };
    return ret;
}

ConstValue createConstF64(CompilerContext* context, double value) {
    ConstValue ret = { .type = createSizedPrimitiveType(&context->types, TYPE_REAL, 64), .f64 = value };
    return ret;
}

ConstValue createConstBool(CompilerContext* context, bool value) {
    ConstValue ret = { .type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL), .boolean = value };
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
            createFormattedString("type error, incompatible types in %s expression, `%S ` and `%S`", getAstPrintName(node->kind), lhs_type, rhs_type), 3,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("types `%S` and `%S` are incompatible", lhs_type, rhs_type), node->location),
            createMessageFragment(MESSAGE_NOTE, createFormattedString("note: lhs has type `%S`", lhs_type), left->location),
            createMessageFragment(MESSAGE_NOTE, createFormattedString("note: rhs has type `%S`", rhs_type), right->location)
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
            createFormattedString("type error, in constant expession, incompatible type `%S` for %s expession", type, getAstPrintName(node->kind)), 1,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("`%S` type not allowed here", type), node->location)
        )
    );
    freeString(type);
    return createConstError(context);
}

#define BACTION_INTS(ACTION)                                \
    if (isSignedIntegerType(left.type) != NULL) {           \
        intmax_t l = left.sint;                             \
        intmax_t r = right.sint;                            \
        res = createConstInt(context, t->size, ACTION);     \
    } else if (isUnsignedIntegerType(left.type) != NULL) {  \
        uintmax_t l = left.uint;                            \
        uintmax_t r = right.uint;                           \
        res = createConstUInt(context, t->size, ACTION);    \
    }

#define BACTION_FLOATS(ACTION)                              \
    if (isFloatType(left.type) != NULL) {                   \
        float l = left.f32;                                 \
        float r = right.f32;                                \
        res = createConstF32(context, ACTION);              \
    } else if (isDoubleType(left.type) != NULL) {           \
        double l = left.f32;                                \
        double r = right.f32;                               \
        res = createConstF64(context, ACTION);              \
    }

#define BACTION_BOOLS(ACTION)                       \
    if (isBooleanType(left.type) != NULL) {         \
        bool l = left.boolean;                      \
        bool r = right.boolean;                     \
        res = createConstBool(context, ACTION);     \
    }

#define BACTIONS_T(ACTIONS)                                             \
    TypeSizedPrimitive* t = (TypeSizedPrimitive*)left.type;             \
    BACTIONS(ACTIONS)

#define BACTIONS(ACTIONS)                                               \
    ACTIONS else {                                                      \
        res = raiseTypeErrorNotInConst(context, n->left, left.type);    \
    }

#define BACTION_INT(ACTION) BACTIONS_T(BACTION_INTS(ACTION))

#define BACTION_NUM(ACTION) BACTIONS_T(BACTION_INTS(ACTION) else BACTION_FLOATS(ACTION))

#define BACTION_ALL(ACTION) BACTIONS_T(BACTION_INTS(ACTION) else BACTION_FLOATS(ACTION) else BACTION_BOOLS(ACTION))

#define BACTION_BOOL(ACTION)                                            \
    BACTION_BOOLS(ACTION) else {                                        \
        res = raiseTypeErrorNotInConst(context, n->left, left.type);    \
    }

#define BINARY_OP(ACTION) {                                                                         \
    AstBinary* n = (AstBinary*)node;                                                                \
    ConstValue left = evaluateConstExpr(context, n->left);                                          \
    ConstValue right = evaluateConstExpr(context, n->right);                                        \
    if (left.type->kind == TYPE_ERROR) {                                                            \
        res = left;                                                                                 \
    } else if (right.type->kind == TYPE_ERROR) {                                                    \
        res = right;                                                                                \
    } else if (!compareStructuralTypes(left.type, right.type)) {                                    \
        res = raiseTypeErrorDifferent(context, node, n->left, n->right, left.type, right.type);     \
    } else { ACTION }                                                                               \
    break;                                                                                          \
}

#define UACTION_INTS(ACTION)                                \
    if (isSignedIntegerType(op.type) != NULL) {             \
        intmax_t o = op.sint;                               \
        res = createConstInt(context, t->size, ACTION);     \
    } else if (isUnsignedIntegerType(op.type) != NULL) {    \
        uintmax_t o = op.uint;                              \
        res = createConstUInt(context, t->size, ACTION);    \
    }

#define UACTION_FLOATS(ACTION)                              \
    if (isFloatType(op.type) != NULL) {                     \
        float o = op.f32;                                   \
        res = createConstF32(context, ACTION);              \
    } else if (isDoubleType(op.type) != NULL) {             \
        double o = op.f32;                                  \
        res = createConstF64(context, ACTION);              \
    }

#define UACTION_BOOLS(ACTION)                       \
    if (isBooleanType(op.type) != NULL) {           \
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
    if (op.type->kind == TYPE_ERROR) {                  \
        res = op;                                       \
    } else { ACTION }                                   \
    break;                                              \
}

ConstValue evaluateConstExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE("should not evaluate");
    } else {
        ConstValue res;
        switch (node->kind) {
            case AST_ERROR: {
                res = createConstError(context);
                break;
            }
            case AST_IF_ELSE: // TODO: if-else expessions?
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
            case AST_ASSIGN:
            case AST_RETURN:
            case AST_ARRAY:
            case AST_VOID:
            case AST_ROOT:
            case AST_LIST:
            case AST_BLOCK:
            case AST_VARDEF: {
                UNREACHABLE("should not evaluate");
            }
            case AST_VAR: // TODO: constant variables? (We need a scope!)
            case AST_INDEX: // TODO: constant arrays?
            case AST_CALL: // TODO: constant calls?
            case AST_STR: // TODO: constant strings?
            case AST_ADDR:
            case AST_DEREF: {
                // None of these are allowed in constant expressions.
                res = raiseOpErrorNotInConst(context, node);
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
            case AST_EQ: BINARY_OP(BACTION_ALL(l == r))
            case AST_NE: BINARY_OP(BACTION_ALL(l != r))
            case AST_LE: BINARY_OP(BACTION_ALL(l <= r))
            case AST_GE: BINARY_OP(BACTION_ALL(l >= r))
            case AST_LT: BINARY_OP(BACTION_ALL(l < r))
            case AST_GT: BINARY_OP(BACTION_ALL(l > r))
            case AST_OR: BINARY_OP(BACTION_BOOL(l || r))
            case AST_AND: BINARY_OP(BACTION_BOOL(l && r))
            case AST_POS: UNARY_OP(UACTION_NUM(o))
            case AST_NEG: UNARY_OP(UACTION_NUM(-o))
            case AST_NOT: UNARY_OP(UACTIONS_T(UACTION_INTS(~o) else UACTION_BOOLS(!o)))
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                if (n->res_type == NULL || n->res_type->kind != TYPE_ERROR) {
                    if (n->res_type == NULL) {
                        n->res_type = createSizedPrimitiveType(&context->types, TYPE_INT, 64);
                    }
                    TypeSizedPrimitive* t = isIntegerType(n->res_type);
                    ASSERT(t != NULL);
                    if (t->kind == TYPE_INT) {
                        res = createConstInt(context, t->size, n->number);
                    } else if (t->kind == TYPE_UINT) {
                        res = createConstUInt(context, t->size, n->number);
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
                if (n->res_type == NULL || n->res_type->kind != TYPE_ERROR) {
                    if (n->res_type == NULL) {
                        n->res_type = createSizedPrimitiveType(&context->types, TYPE_REAL, 64);
                    }
                    TypeSizedPrimitive* t = isRealType(n->res_type);
                    ASSERT(t != NULL);
                    if (t->size == 32) {
                        res = createConstF32(context, n->number);
                    } else if (t->size == 64) {
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
                if (n->res_type == NULL || n->res_type->kind != TYPE_ERROR) {
                    if (n->res_type == NULL) {
                        n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_BOOL);
                    }
                    Type* t = isBooleanType(n->res_type);
                    ASSERT(t != NULL);
                    if (t->kind == TYPE_INT) {
                        res = createConstBool(context, n->value);
                    } else {
                        UNREACHABLE("integer type expected");
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

