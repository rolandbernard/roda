
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
        {
            /* ConstValue* res = NULL; */
            /* switch (node->kind) { */
            /*     case AST_ERROR: { */
            /*         res = createConstError(context); */
            /*         break; */
            /*     } */
            /*     case AST_VAR: { */
            /*         AstVar* n = (AstVar*)node; */
            /*         SymbolVariable* var = (SymbolVariable*)n->binding; */
            /*         if (var->value.type == RECURSION_CHECK_TYPE) { */
            /*             addMessageToContext(&context->msgs, createMessage(ERROR_INVALID_CONST, */
            /*                 createFormattedString("recursive reference in constant definition of `%s`", var->name), 1, */
            /*                 createMessageFragment(MESSAGE_ERROR, copyFromCString("recursive reference to constant"), node->location) */
            /*             )); */
            /*             res = createConstError(context); */
            /*         } else { */
            /*             if (var->value.type == NULL) { */
            /*                 evaluateConstantDefinition(context, (AstVarDef*)var->def->parent); */
            /*             } */
            /*             res = var->value; */
            /*         } */
            /*         break; */
            /*     } */
            /*     case AST_BLOCK_EXPR: { */
            /*         AstBlock* n = (AstBlock*)node; */
            /*         for (size_t i = 0; i < n->nodes->count; i++) { */
            /*             res = evaluateConstExpr(context, n->nodes->nodes[i]); */
            /*         } */
            /*         break; */
            /*     } */
            /*     case AST_IF_ELSE_EXPR: { */
            /*         AstIfElse* n = (AstIfElse*)node; */
            /*         ConstValue* cond = evaluateConstExpr(context, n->condition); */
            /*         if (cond->kind == CONST_BOOL) { */
            /*             if (cond.boolean) { */
            /*                 res = evaluateConstExpr(context, n->if_block); */
            /*             } else { */
            /*                 res = evaluateConstExpr(context, n->else_block); */
            /*             } */
            /*         } else { */
            /*             res = raiseTypeErrorNotInConst(context, n->condition, n->condition->res_type); */
            /*         } */
            /*         break; */
            /*     } */
            /*     case AST_AS: { */
            /*         AstBinary* n = (AstBinary*)node; */
            /*         if (n->res_type == NULL) { */
            /*             n->res_type = evaluateTypeExpr(context, n->right); */
            /*         } */
            /*         ConstValue* op = evaluateConstExpr(context, n->left); */
            /*         size_t size = getIntRealTypeSize(n->res_type); */
            /*         if (isRealType(n->res_type)) { */
            /*             size_t op_size = getIntRealTypeSize(n->left->res_type); */
            /*             if (isRealType(n->left->res_type)) { */
            /*                 if (size == op_size) { */
            /*                     res = op; */
            /*                     break; */
            /*                 } else if (size == 32) { */
            /*                     res = createConstF32(context, (float)op.f64); */
            /*                     break; */
            /*                 } else if (size == 64) { */
            /*                     res = createConstF64(context, (double)op.f32); */
            /*                     break; */
            /*                 } */
            /*             } else if (isSignedIntegerType(n->left->res_type)) { */
            /*                 if (size == 32) { */
            /*                     res = createConstF32(context, (float)op.sint); */
            /*                     break; */
            /*                 } else if (size == 64) { */
            /*                     res = createConstF64(context, (double)op.sint); */
            /*                     break; */
            /*                 } */
            /*             } else if (isUnsignedIntegerType(n->left->res_type)) { */
            /*                 if (size == 32) { */
            /*                     res = createConstF32(context, (float)op.uint); */
            /*                     break; */
            /*                 } else if (size == 64) { */
            /*                     res = createConstF64(context, (double)op.uint); */
            /*                     break; */
            /*                 } */
            /*             } */
            /*         } else if (isSignedIntegerType(n->res_type)) { */
            /*             if (isFloatType(n->left->res_type)) { */
            /*                 res = createConstInt(context, size, (intmax_t)op.f32); */
            /*                 break; */
            /*             } else if (isDoubleType(n->left->res_type)) { */
            /*                 res = createConstInt(context, size, (intmax_t)op.f64); */
            /*                 break; */
            /*             } else if (isSignedIntegerType(n->left->res_type)) { */
            /*                 res = createConstInt(context, size, (intmax_t)op.sint); */
            /*                 break; */
            /*             } else if (isUnsignedIntegerType(n->left->res_type)) { */
            /*                 res = createConstInt(context, size, (intmax_t)op.uint); */
            /*                 break; */
            /*             } */
            /*         } else if (isUnsignedIntegerType(n->res_type)) { */
            /*             if (isFloatType(n->left->res_type)) { */
            /*                 res = createConstUInt(context, size, (uintmax_t)op.f32); */
            /*                 break; */
            /*             } else if (isDoubleType(n->left->res_type)) { */
            /*                 res = createConstUInt(context, size, (uintmax_t)op.f64); */
            /*                 break; */
            /*             } else if (isSignedIntegerType(n->left->res_type)) { */
            /*                 res = createConstUInt(context, size, (uintmax_t)op.sint); */
            /*                 break; */
            /*             } else if (isUnsignedIntegerType(n->left->res_type)) { */
            /*                 res = createConstUInt(context, size, (uintmax_t)op.uint); */
            /*                 break; */
            /*             } */
            /*         } */
            /*         res = raiseTypeErrorNotInConst(context, n->left, n->left->res_type); */
            /*         break; */
            /*     } */
            /*     case AST_ADD: BINARY_OP(BACTION_NUM(l + r)) */
            /*     case AST_SUB: BINARY_OP(BACTION_NUM(l - r)) */
            /*     case AST_MUL: BINARY_OP(BACTION_NUM(l * r)) */
            /*     case AST_DIV: BINARY_OP(BACTION_NUM(l / r)) */
            /*     case AST_MOD: BINARY_OP(BACTION_INT(l % r)) */
            /*     case AST_SHL: BINARY_OP(BACTION_INT(l << r)) */
            /*     case AST_SHR: BINARY_OP(BACTION_INT(l >> r)) */
            /*     case AST_BAND: BINARY_OP(BACTION_INT(l & r)) */
            /*     case AST_BOR: BINARY_OP(BACTION_INT(l | r)) */
            /*     case AST_BXOR: BINARY_OP(BACTION_INT(l ^ r)) */
            /*     case AST_EQ: BINARY_OP(BACTION_CMP(l == r)) */
            /*     case AST_NE: BINARY_OP(BACTION_CMP(l != r)) */
            /*     case AST_LE: BINARY_OP(BACTION_CMP(l <= r)) */
            /*     case AST_GE: BINARY_OP(BACTION_CMP(l >= r)) */
            /*     case AST_LT: BINARY_OP(BACTION_CMP(l < r)) */
            /*     case AST_GT: BINARY_OP(BACTION_CMP(l > r)) */
            /*     case AST_OR: BINARY_OP(BACTION_BOOL(l || r)) */
            /*     case AST_AND: BINARY_OP(BACTION_BOOL(l && r)) */
            /*     case AST_POS: UNARY_OP(UACTION_NUM(o)) */
            /*     case AST_NEG: UNARY_OP(UACTION_NUM(-o)) */
            /*     case AST_NOT: UNARY_OP(UACTIONS_T(UACTION_INTS(~o) else UACTION_BOOLS(!o))) */
            /*     case AST_CHAR: */
            /*     case AST_INT: { */
            /*         AstInt* n = (AstInt*)node; */
            /*         if (!isErrorType(n->res_type)) { */
            /*             if (n->res_type == NULL) { */
            /*                 n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_INT, 64); */
            /*             } */
            /*             if (isSignedIntegerType(n->res_type)) { */
            /*                 res = createConstInt(context, getIntRealTypeSize(n->res_type), n->number); */
            /*             } else if (isUnsignedIntegerType(n->res_type)) { */
            /*                 res = createConstUInt(context, getIntRealTypeSize(n->res_type), n->number); */
            /*             } else { */
            /*                 UNREACHABLE("integer type expected"); */
            /*             } */
            /*         } else { */
            /*             res = createConstError(context); */
            /*         } */
            /*         break; */
            /*     } */
            /*     case AST_REAL: { */
            /*         AstReal* n = (AstReal*)node; */
            /*         if (!isErrorType(n->res_type)) { */
            /*             if (n->res_type == NULL) { */
            /*                 n->res_type = createSizedPrimitiveType(&context->types, NULL, TYPE_REAL, 64); */
            /*             } */
            /*             if (isFloatType(n->res_type)) { */
            /*                 res = createConstF32(context, n->number); */
            /*             } else if (isDoubleType(n->res_type)) { */
            /*                 res = createConstF64(context, n->number); */
            /*             } else { */
            /*                 UNREACHABLE("unexpected real type size"); */
            /*             } */
            /*         } else { */
            /*             res = createConstError(context); */
            /*         } */
            /*         break; */
            /*     } */
            /*     case AST_BOOL: { */
            /*         AstBool* n = (AstBool*)node; */
            /*         if (!isErrorType(n->res_type)) { */
            /*             if (n->res_type == NULL) { */
            /*                 n->res_type = createUnsizedPrimitiveType(&context->types, NULL, TYPE_BOOL); */
            /*             } */
            /*             if (isBooleanType(n->res_type)) { */
            /*                 res = createConstBool(context, n->value); */
            /*             } else { */
            /*                 UNREACHABLE("boolean type expected"); */
            /*             } */
            /*         } else { */
            /*             res = createConstError(context); */
            /*         } */
            /*         break; */
            /*     } */
            /*     default: */
            /*         UNREACHABLE("should not evaluate"); */
            /* } */
            /* node->res_type = res.type; */
            /* return res; */
        }
        UNREACHABLE("TODO");
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

