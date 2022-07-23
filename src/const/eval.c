
#include "types/eval.h"
#include "ast/astprinter.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "types/check.h"
#include "types/infer.h"

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
                        createFormattedString("recursive reference in constant definition of `%s`", var->name), 1,
                        createMessageFragment(MESSAGE_ERROR, copyFromCString("recursive reference to constant"), node->location)
                    ));
                } else {
                    if (var->value == NULL) {
                        evaluateConstantDefinition(context, (AstVarDef*)var->def->parent);
                    }
                    res = copyConstValue(var->value);
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
                ASSERT(cond->kind == CONST_BOOL);
                if (cond->val) {
                    res = evaluateConstExpr(context, n->if_block);
                } else {
                    res = evaluateConstExpr(context, n->else_block);
                }
                freeConstValue((ConstValue*)cond);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                ConstValue* op = evaluateConstExpr(context, n->left);
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
                    FixedInt* value = NULL;
                    if (op->kind == CONST_DOUBLE) {
                        value = createFixedIntFromDouble(size, ((ConstValueDouble*)op)->val);
                    } else if (op->kind == CONST_FLOAT) {
                        value = createFixedIntFromDouble(size, ((ConstValueFloat*)op)->val);
                    } else if (op->kind == CONST_INT) {
                        value = resizeFixedIntSignExtend(((ConstValueInt*)op)->val, size);
                    } else if (op->kind == CONST_UINT) {
                        value = resizeFixedIntZeroExtend(((ConstValueInt*)op)->val, size);
                    } else {
                        UNREACHABLE("should have correct type");
                    }
                    freeConstValue(op);
                    if (isSignedIntegerType(n->res_type)) {
                        res = createConstInt(n->res_type, value);
                    } else {
                        res = createConstUint(n->res_type, value);
                    }
                } else {
                    UNREACHABLE("should have correct type");
                }
                break;
            }
            /* case AST_ADD: BINARY_OP(BACTION_NUM(l + r)) */
            /* case AST_SUB: BINARY_OP(BACTION_NUM(l - r)) */
            /* case AST_MUL: BINARY_OP(BACTION_NUM(l * r)) */
            /* case AST_DIV: BINARY_OP(BACTION_NUM(l / r)) */
            /* case AST_MOD: BINARY_OP(BACTION_INT(l % r)) */
            /* case AST_SHL: BINARY_OP(BACTION_INT(l << r)) */
            /* case AST_SHR: BINARY_OP(BACTION_INT(l >> r)) */
            /* case AST_BAND: BINARY_OP(BACTION_INT(l & r)) */
            /* case AST_BOR: BINARY_OP(BACTION_INT(l | r)) */
            /* case AST_BXOR: BINARY_OP(BACTION_INT(l ^ r)) */
            /* case AST_EQ: BINARY_OP(BACTION_CMP(l == r)) */
            /* case AST_NE: BINARY_OP(BACTION_CMP(l != r)) */
            /* case AST_LE: BINARY_OP(BACTION_CMP(l <= r)) */
            /* case AST_GE: BINARY_OP(BACTION_CMP(l >= r)) */
            /* case AST_LT: BINARY_OP(BACTION_CMP(l < r)) */
            /* case AST_GT: BINARY_OP(BACTION_CMP(l > r)) */
            /* case AST_OR: BINARY_OP(BACTION_BOOL(l || r)) */
            /* case AST_AND: BINARY_OP(BACTION_BOOL(l && r)) */
            /* case AST_POS: UNARY_OP(UACTION_NUM(o)) */
            /* case AST_NEG: UNARY_OP(UACTION_NUM(-o)) */
            /* case AST_NOT: UNARY_OP(UACTIONS_T(UACTION_INTS(~o) else UACTION_BOOLS(!o))) */
            /* case AST_CHAR: */
            /* case AST_INT: { */
            /*     AstInt* n = (AstInt*)node; */
            /*     if (!isErrorType(n->res_type)) { */
            /*         if (n->res_type == NULL) { */
            /*             n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_INT, 64); */
            /*         } */
            /*         if (isSignedIntegerType(n->res_type)) { */
            /*             res = createConstInt(context, getIntRealTypeSize(n->res_type), n->number); */
            /*         } else if (isUnsignedIntegerType(n->res_type)) { */
            /*             res = createConstUInt(context, getIntRealTypeSize(n->res_type), n->number); */
            /*         } else { */
            /*             UNREACHABLE("integer type expected"); */
            /*         } */
            /*     } else { */
            /*         res = createConstError(context); */
            /*     } */
            /*     break; */
            /* } */
            /* case AST_REAL: { */
            /*     AstReal* n = (AstReal*)node; */
            /*     if (!isErrorType(n->res_type)) { */
            /*         if (n->res_type == NULL) { */
            /*             n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_REAL, 64); */
            /*         } */
            /*         if (isFloatType(n->res_type)) { */
            /*             res = createConstF32(context, n->number); */
            /*         } else if (isDoubleType(n->res_type)) { */
            /*             res = createConstF64(context, n->number); */
            /*         } else { */
            /*             UNREACHABLE("unexpected real type size"); */
            /*         } */
            /*     } else { */
            /*         res = createConstError(context); */
            /*     } */
            /*     break; */
            /* } */
            /* case AST_BOOL: { */
            /*     AstBool* n = (AstBool*)node; */
            /*     if (!isErrorType(n->res_type)) { */
            /*         if (n->res_type == NULL) { */
            /*             n->res_type = createUnsizedPrimitiveType(&context->types, NULL, TYPE_BOOL); */
            /*         } */
            /*         if (isBooleanType(n->res_type)) { */
            /*             res = createConstBool(context, n->value); */
            /*         } else { */
            /*             UNREACHABLE("boolean type expected"); */
            /*         } */
            /*     } else { */
            /*         res = createConstError(context); */
            /*     } */
            /*     break; */
            /* } */
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
            case AST_CHAR:
            case AST_INT:
            case AST_REAL:
            case AST_BOOL: {
                return true;
            }
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
    size_t old_error = context->msgs.error_count;
    if (checkValidInConstExpr(context, def->val)) {
        typeInferExpr(context, (AstNode*)def, NULL);
        typeCheckExpr(context, (AstNode*)def);
    }
    if (context->msgs.error_count == old_error && def->name->binding != NULL) {
        SymbolVariable* var = (SymbolVariable*)def->name->binding;
        var->value = RECURSION_CHECK_TYPE;
        var->value = evaluateConstExpr(context, def->val);
    }
}

