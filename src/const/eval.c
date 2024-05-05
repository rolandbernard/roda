
#include "ast/astprinter.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "types/check.h"
#include "types/infer.h"
#include "util/alloc.h"

#include "const/eval.h"

static void raiseOpErrorNotInConst(CompilerContext* context, AstNode* node) {
    const char* ast_name = getAstPrintName(node->kind);
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_NOT_CONSTEXPR,
            createFormattedString("%s expression not allowed in constant expressions", ast_name), 1,
            createMessageFragment(MESSAGE_ERROR, createFormattedString("%s not allowed here", ast_name), node->location)
        )
    );
}

#define RECURSION_CHECK_TYPE ((ConstValue*)-1)

#define BOP_INT_A(ACTION)                               \
    if (left->kind == CONST_INT) {                      \
        FixedInt* a = ((ConstValueInt*)left)->val;      \
        FixedInt* b = ((ConstValueInt*)right)->val;     \
        ACTION                                          \
    }
#define BOP_INT(ACTION) \
    BOP_INT_A(res = createConstInt(node->res_type, ACTION);)

#define BOP_UINT_A(ACTION)                              \
    if (left->kind == CONST_UINT) {                     \
        FixedInt* a = ((ConstValueInt*)left)->val;      \
        FixedInt* b = ((ConstValueInt*)right)->val;     \
        ACTION                                          \
    }
#define BOP_UINT(ACTION) \
    BOP_UINT_A(res = createConstUint(node->res_type, ACTION);)

#define BOP_FIXED_INT(ACTION) BOP_INT(ACTION) else BOP_UINT(ACTION)
#define BOP_FIXED_INT_S(ACTION) BOP_INT(s ## ACTION (a, b)) else BOP_UINT(u ## ACTION (a, b))

#define BOP_BIG_INT_A(ACTION)                           \
    if (left->kind == CONST_BIG_INT) {                  \
        BigInt* a = ((ConstValueBigInt*)left)->val;     \
        BigInt* b = ((ConstValueBigInt*)right)->val;    \
        ACTION                                          \
    }
#define BOP_BIG_INT(ACTION) \
    BOP_BIG_INT_A(res = createConstBigInt(node->res_type, ACTION);)

#define BOP_ANY_INT(ACTION) BOP_FIXED_INT(ACTION ## FixedInt (a, b)) else BOP_BIG_INT(ACTION ## BigInt (a, b))
#define BOP_ANY_INT_S(ACTION) BOP_FIXED_INT_S(ACTION ## FixedInt) else BOP_BIG_INT(ACTION ## BigInt (a, b))

#define BOP_DOUBLE_A(ACTION)                            \
    if (left->kind == CONST_DOUBLE) {                   \
        double a = ((ConstValueDouble*)left)->val;      \
        double b = ((ConstValueDouble*)right)->val;     \
        ACTION                                          \
    }
#define BOP_DOUBLE(ACTION) \
    BOP_DOUBLE_A(res = createConstDouble(node->res_type, ACTION);)

#define BOP_FLOAT_A(ACTION)                             \
    if (left->kind == CONST_FLOAT) {                    \
        float a = ((ConstValueFloat*)left)->val;        \
        float b = ((ConstValueFloat*)right)->val;       \
        ACTION                                          \
    }
#define BOP_FLOAT(ACTION) \
    BOP_FLOAT_A(res = createConstFloat(node->res_type, ACTION);)

#define BOP_REAL(ACTION) BOP_DOUBLE(ACTION) else BOP_FLOAT(ACTION)

#define BOP_BOOL_A(ACTION)                              \
    if (left->kind == CONST_BOOL) {                     \
        bool a = ((ConstValueBool*)left)->val;          \
        bool b = ((ConstValueBool*)right)->val;         \
        ACTION                                          \
    }
#define CREATE_BOOL(ACTION) res = createConstBool(node->res_type, ACTION);
#define BOP_BOOL(ACTION) \
    BOP_BOOL_A(CREATE_BOOL(ACTION))

#define BOP_CMP(REL)                                                    \
    BOP_INT_A(CREATE_BOOL(compareFixedIntSigned(a, b) REL 0))           \
    else BOP_UINT_A(CREATE_BOOL(compareFixedIntUnsigned(a, b) REL 0))   \
    else BOP_BIG_INT_A(CREATE_BOOL(compareBigInt(a, b) REL 0))          \
    else BOP_DOUBLE_A(CREATE_BOOL(a REL b))                             \
    else BOP_FLOAT_A(CREATE_BOOL(a REL b))

#define BINARY_OP(OPS) {                                                    \
    AstBinary* n = (AstBinary*)node;                                        \
    ASSERT(compareStructuralTypes(n->left->res_type, n->right->res_type));  \
    ConstValue* left = evaluateConstExpr(context, n->left);                 \
    if (left != NULL) {                                                     \
        ConstValue* right = evaluateConstExpr(context, n->right);           \
        if (right != NULL) {                                                \
            ASSERT(left->kind == right->kind);                              \
            OPS                                                             \
            freeConstValue(right);                                          \
        }                                                                   \
        freeConstValue(left);                                               \
    }                                                                       \
    break;                                                                  \
}

ConstValue* evaluateConstExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE("should not evaluate");
    } else {
        ASSERT(node->res_type != NULL /* Type check first */);
        ConstValue* res = NULL;
        switch (node->kind) {
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                SymbolVariable* var = (SymbolVariable*)n->binding;
                if (var->value == RECURSION_CHECK_TYPE) {
                    addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_CONST,
                        createFormattedString("in constant expression, recursive constant reference to `%s`", var->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("recursive reference to constant"), node->location)
                    ));
                } else {
                    evaluateConstantDefinition(context, (AstVarDef*)var->def->parent);
                    if (var->value != NULL) {
                        res = copyConstValue(var->value);
                    }
                }
                break;
            }
            case AST_BLOCK_EXPR: {
                AstBlock* n = (AstBlock*)node;
                for (size_t i = 0; i < n->nodes->count; i++) {
                    freeConstValue(res);
                    res = evaluateConstExpr(context, n->nodes->nodes[i]);
                }
                break;
            }
            case AST_IF_ELSE_EXPR: {
                AstIfElse* n = (AstIfElse*)node;
                ConstValueBool* cond = (ConstValueBool*)evaluateConstExpr(context, n->condition);
                if (cond != NULL) {
                    ASSERT(cond->kind == CONST_BOOL);
                    if (cond->val) {
                        res = evaluateConstExpr(context, n->if_block);
                    } else {
                        res = evaluateConstExpr(context, n->else_block);
                    }
                    freeConstValue((ConstValue*)cond);
                }
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                ConstValue* op = evaluateConstExpr(context, n->left);
                if (op != NULL) {
                    size_t size = getIntRealTypeSize(n->res_type);
                    if (compareStructuralTypes(op->type, n->res_type)) {
                        res = op;
                    } else if (isRealType(n->res_type)) {
                        double value = 0;
                        if (op->kind == CONST_DOUBLE) {
                            value = ((ConstValueDouble*)op)->val;
                        } else if (op->kind == CONST_FLOAT) {
                            value = ((ConstValueFloat*)op)->val;
                        } else if (op->kind == CONST_INT) {
                            value = doubleForFixedIntSigned(((ConstValueInt*)op)->val);
                        } else if (op->kind == CONST_UINT) {
                            value = doubleForFixedIntUnsigned(((ConstValueInt*)op)->val);
                        } else if (op->kind == CONST_BIG_INT) {
                            value = doubleForFixedIntUnsigned(((ConstValueInt*)op)->val);
                        } else {
                            UNREACHABLE("should have correct type");
                        }
                        freeConstValue(op);
                        if (size == 32) {
                            res = createConstFloat(n->res_type, value);
                        } else if (size == 64) {
                            res = createConstDouble(n->res_type, value);
                        }
                    } else if (isIntegerType(n->res_type)) {
                        if (size == SIZE_SIZE || size == 0) {
                            BigInt* value = NULL;
                            if (op->kind == CONST_DOUBLE) {
                                value = createBigIntFromDouble(((ConstValueDouble*)op)->val);
                            } else if (op->kind == CONST_FLOAT) {
                                value = createBigIntFromDouble(((ConstValueFloat*)op)->val);
                            } else if (op->kind == CONST_INT) {
                                value = createBigIntFromFixedIntSignExtend(((ConstValueInt*)op)->val);
                            } else if (op->kind == CONST_UINT) {
                                value = createBigIntFromFixedIntZeroExtend(((ConstValueInt*)op)->val);
                            } else if (op->kind == CONST_BIG_INT) {
                                value = copyBigInt(((ConstValueBigInt*)op)->val);
                            } else {
                                UNREACHABLE("should have correct type");
                            }
                            freeConstValue(op);
                            res = createConstBigInt(n->res_type, value);
                        } else {
                            FixedInt* value = NULL;
                            if (op->kind == CONST_DOUBLE) {
                                value = createFixedIntFromDouble(size, ((ConstValueDouble*)op)->val);
                            } else if (op->kind == CONST_FLOAT) {
                                value = createFixedIntFromDouble(size, ((ConstValueFloat*)op)->val);
                            } else if (op->kind == CONST_INT) {
                                value = resizeFixedIntSignExtend(((ConstValueInt*)op)->val, size);
                            } else if (op->kind == CONST_UINT) {
                                value = resizeFixedIntZeroExtend(((ConstValueInt*)op)->val, size);
                            } else if (op->kind == CONST_BIG_INT) {
                                value = createFixedIntFromBigInt(size, ((ConstValueBigInt*)op)->val);
                            } else {
                                UNREACHABLE("should have correct type");
                            }
                            freeConstValue(op);
                            if (isSignedIntegerType(n->res_type)) {
                                res = createConstInt(n->res_type, value);
                            } else {
                                res = createConstUint(n->res_type, value);
                            }
                        }
                    } else {
                        UNREACHABLE("should have correct type");
                    }
                }
                break;
            }
            case AST_ADD: BINARY_OP(BOP_ANY_INT(add) else BOP_REAL(a + b))
            case AST_SUB: BINARY_OP(BOP_ANY_INT(sub) else BOP_REAL(a - b))
            case AST_MUL: BINARY_OP(BOP_ANY_INT(mul) else BOP_REAL(a * b))
            case AST_DIV: BINARY_OP(BOP_ANY_INT_S(div) else BOP_REAL(a / b))
            case AST_MOD: BINARY_OP(BOP_ANY_INT_S(rem))
            case AST_SHL: BINARY_OP(
                BOP_BIG_INT(shiftLeftBigInt(a, uintMaxForBigInt(b)))
                else BOP_FIXED_INT(shiftLeftFixedInt(a, uintMaxForFixedInt(b)))
            )
            case AST_SHR: BINARY_OP(
                BOP_BIG_INT(shiftRightBigInt(a, uintMaxForBigInt(b)))
                else BOP_INT(shiftRightArithmeticFixedInt(a, uintMaxForFixedInt(b)))
                else BOP_UINT(shiftRightLogicalFixedInt(a, uintMaxForFixedInt(b)))
            )
            case AST_BAND: BINARY_OP(BOP_ANY_INT(and))
            case AST_BOR: BINARY_OP(BOP_ANY_INT(or))
            case AST_BXOR: BINARY_OP(BOP_ANY_INT(xor))
            case AST_EQ: BINARY_OP(BOP_CMP(==) else BOP_BOOL(a == b))
            case AST_NE: BINARY_OP(BOP_CMP(!=) else BOP_BOOL(a != b))
            case AST_LE: BINARY_OP(BOP_CMP(<=))
            case AST_GE: BINARY_OP(BOP_CMP(>=))
            case AST_LT: BINARY_OP(BOP_CMP(<))
            case AST_GT: BINARY_OP(BOP_CMP(>))
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                ASSERT(compareStructuralTypes(n->left->res_type, n->right->res_type));
                ConstValue* left = evaluateConstExpr(context, n->left);
                if (left != NULL) {
                    ASSERT(left->kind == CONST_BOOL);
                    if (
                        (n->kind == AST_AND && !((ConstValueBool*)left)->val)
                        || (n->kind == AST_OR && ((ConstValueBool*)left)->val)
                    ) {
                        res = createConstBool(node->res_type, ((ConstValueBool*)left)->val);
                    } else {
                        ConstValue* right = evaluateConstExpr(context, n->right);
                        if (right != NULL) {
                            ASSERT(right->kind == CONST_BOOL);
                            res = createConstBool(node->res_type, ((ConstValueBool*)right)->val);
                            freeConstValue(right);
                        }
                    }
                    freeConstValue(left);
                }
                break;
            }
            case AST_POS: {
                AstUnary* n = (AstUnary*)node;
                res = evaluateConstExpr(context, n->op);
                break;
            }
            case AST_NEG: {
                AstUnary* n = (AstUnary*)node;
                ConstValue* op = evaluateConstExpr(context, n->op);
                if (op != NULL) {
                    if (op->kind == CONST_INT) {
                        FixedInt* v = ((ConstValueInt*)op)->val;
                        res = createConstInt(node->res_type, negFixedInt(v));
                    } else if (op->kind == CONST_BIG_INT) {
                        BigInt* v = ((ConstValueBigInt*)op)->val;
                        res = createConstBigInt(node->res_type, negBigInt(v));
                    } else if (op->kind == CONST_DOUBLE) {
                        double v = ((ConstValueDouble*)op)->val;
                        res = createConstDouble(node->res_type, -v);
                    } else if (op->kind == CONST_FLOAT) {
                        float v = ((ConstValueFloat*)op)->val;
                        res = createConstFloat(node->res_type, -v);
                    }
                    freeConstValue(op);
                }
                break;
            }
            case AST_NOT: {
                AstUnary* n = (AstUnary*)node;
                ConstValue* op = evaluateConstExpr(context, n->op);
                if (op != NULL) {
                    if (op->kind == CONST_INT) {
                        FixedInt* v = ((ConstValueInt*)op)->val;
                        res = createConstInt(node->res_type, notFixedInt(v));
                    } else if (op->kind == CONST_UINT) {
                        FixedInt* v = ((ConstValueInt*)op)->val;
                        res = createConstUint(node->res_type, notFixedInt(v));
                    } else if (op->kind == CONST_BIG_INT) {
                        BigInt* v = ((ConstValueBigInt*)op)->val;
                        res = createConstBigInt(node->res_type, notBigInt(v));
                    }
                    freeConstValue(op);
                }
                break;
            }
            case AST_CHAR:
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                size_t size = getIntRealTypeSize(n->res_type);
                if (size == SIZE_SIZE || size == 0) {
                    res = createConstBigInt(node->res_type, copyBigInt(n->number));
                } else if (isSignedIntegerType(n->res_type)) {
                    res = createConstInt(node->res_type, createFixedIntFromBigInt(size, n->number));
                } else if (isUnsignedIntegerType(n->res_type)) {
                    res = createConstUint(node->res_type, createFixedIntFromBigInt(size, n->number));
                }
                break;
            }
            case AST_REAL: {
                AstReal* n = (AstReal*)node;
                if (isFloatType(n->res_type)) {
                    res = createConstFloat(n->res_type, n->number);
                } else if (isDoubleType(n->res_type)) {
                    res = createConstDouble(n->res_type, n->number);
                }
                break;
            }
            case AST_BOOL: {
                AstBool* n = (AstBool*)node;
                res = createConstBool(node->res_type, n->value);
                break;
            }
            case AST_VOID: {
                res = createConstVoid(node->res_type);
                break;
            }
            case AST_ARRAY_LIT:
            case AST_TUPLE_LIT: {
                AstList* n = (AstList*)node;
                ConstValue** vals = ALLOC(ConstValue*, n->count);
                for (size_t i = 0; i < n->count; i++) {
                    vals[i] = evaluateConstExpr(context, n->nodes[i]);
                    if (vals[i] == NULL) {
                        for (size_t j = 0; j < i; j++) {
                            freeConstValue(vals[j]);
                        }
                        FREE(vals);
                        return NULL;
                    }
                }
                if (n->kind == AST_ARRAY_LIT) {
                    res = createConstArray(n->res_type, vals, n->count);
                } else {
                    res = createConstTuple(n->res_type, vals, n->count);
                }
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                Symbol* names = ALLOC(Symbol, n->count);
                ConstValue** vals = ALLOC(ConstValue*, n->count);
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* f = (AstStructField*)n->nodes[i];
                    names[i] = f->name->name;
                    vals[i] = evaluateConstExpr(context, f->field_value);
                    if (vals[i] == NULL) {
                        for (size_t j = 0; j < i; j++) {
                            freeConstValue(vals[j]);
                        }
                        FREE(vals);
                        FREE(names);
                        return NULL;
                    }
                }
                res = createConstStruct(n->res_type, names, vals, n->count);
                break;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                ConstValue* arr = evaluateConstExpr(context, n->left);
                if (arr != NULL) {
                    ASSERT(arr->kind == CONST_ARRAY);
                    ConstValue* index = evaluateConstExpr(context, n->right);
                    if (index != NULL) {
                        size_t idx;
                        if (index->kind == CONST_BIG_INT) {
                            idx = intMaxForBigInt(((ConstValueBigInt*)index)->val);
                        } else {
                            idx = uintMaxForFixedInt(((ConstValueInt*)index)->val);
                        }
                        ConstValueList* list = (ConstValueList*)arr;
                        if (idx < list->count) {
                            res = copyConstValue(list->values[idx]);
                        } else {
                            String type = buildTypeName(arr->type);
                            addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_CONST,
                                createFormattedString("in constant expession, array index `%zu` out of bounds for type `%s`", idx, cstr(type)), 1,
                                createMessageFragment(MESSAGE_ERROR, copyFromCString("index out of bounds"), n->right->location)
                            ));
                            freeString(type);
                        }
                        freeConstValue(index);
                    }
                    freeConstValue(arr);
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                ConstValue* value = evaluateConstExpr(context, n->strct);
                if (value != NULL) {
                    ASSERT(value->kind == CONST_STRUCT);
                    ConstValueStruct* strct = (ConstValueStruct*)value;
                    res = copyConstValue(getValueFromConstStruct(strct, n->field->name));
                    freeConstValue(value);
                }
                break;
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                ConstValue* value = evaluateConstExpr(context, n->tuple);
                if (value != NULL) {
                    ASSERT(value->kind == CONST_TUPLE);
                    ConstValueList* list = (ConstValueList*)value;
                    size_t idx = uintMaxForBigInt(n->field->number);
                    res = copyConstValue(list->values[idx]);
                    freeConstValue(value);
                }
                break;
            }
            case AST_CONSTDEF:
                break;
            default:
                UNREACHABLE("should not evaluate");
        }
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
            case AST_VOID:
            case AST_CHAR:
            case AST_INT:
            case AST_REAL:
            case AST_BOOL: {
                return true;
            }
            case AST_ARRAY_LIT:
            case AST_TUPLE_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    if (!checkValidInConstExpr(context, n->nodes[i])) {
                        return false;
                    }
                }
                return true;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* f = (AstStructField*)n->nodes[i];
                    if (!checkValidInConstExpr(context, f->field_value)) {
                        return false;
                    }
                }
                return true;
            }
            case AST_INDEX: {
                AstBinary* n = (AstBinary*)node;
                return checkValidInConstExpr(context, n->left)
                    && checkValidInConstExpr(context, n->right);
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                return checkValidInConstExpr(context, n->strct);
            }
            case AST_TUPLE_INDEX: {
                AstTupleIndex* n = (AstTupleIndex*)node;
                return checkValidInConstExpr(context, n->tuple);
            }
            case AST_CONSTDEF:
                return true;
            default: {
                // None of these are allowed in constant expressions.
                raiseOpErrorNotInConst(context, node);
                return false;
            }
        }
        return false;
    }
}

void evaluateConstantDefinition(CompilerContext* context, AstVarDef* def) {
    if (def->name->binding != NULL) {
        SymbolVariable* var = (SymbolVariable*)def->name->binding;
        if (!var->evaluated) {
            size_t old_error = context->msgs.error_count;
            if (checkValidInConstExpr(context, def->val)) {
                typeInferExpr(context, (AstNode*)def, NULL);
                typeCheckExpr(context, (AstNode*)def);
            }
            if (context->msgs.error_count == old_error) {
                var->value = RECURSION_CHECK_TYPE;
                var->value = evaluateConstExpr(context, def->val);
                var->evaluated = true;
            }
        }
    }
}

