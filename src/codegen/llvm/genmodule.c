
#include <stdbool.h>
#include <string.h>

#include "codegen/llvm/gentype.h"
#include "errors/fatalerror.h"
#include "util/alloc.h"

#include "codegen/llvm/genmodule.h"

typedef struct {
    LLVMValueRef value;
    bool is_reference;
} LlvmCodegenValue;

static LlvmCodegenValue createLlvmCodegenValue(LLVMValueRef value, bool is_reference) {
    LlvmCodegenValue ret = { .value = value, .is_reference = is_reference };
    return ret;
}

typedef struct {
    LLVMModuleRef module;
    LLVMValueRef ret_value;
    LLVMBasicBlockRef exit;
    LLVMBuilderRef builder;
} LlvmCodegenBodyContext;

static LlvmCodegenValue buildFunctionBody(LlvmCodegenContext* context, LlvmCodegenBodyContext* data, AstNode* node);

static LLVMValueRef getCodegenValue(LlvmCodegenContext* context, LlvmCodegenBodyContext* data, AstNode* node) {
    LlvmCodegenValue value = buildFunctionBody(context, data, node);
    if (value.is_reference) {
        return LLVMBuildLoad2(data->builder, generateLlvmType(context, node->res_type), value.value, "tmp");
    } else {
        return value.value;
    }
}

static LLVMValueRef getCodegenReference(LlvmCodegenContext* context, LlvmCodegenBodyContext* data, AstNode* node) {
    LlvmCodegenValue value = buildFunctionBody(context, data, node);
    ASSERT(value.is_reference);
    return value.value;
}

static LLVMValueRef buildBinaryOperation(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, Type* type, AstNodeKind kind) {
    switch (kind) {
        case AST_ADD:
            if (isRealType(type) != NULL) {
                return LLVMBuildFAdd(builder, lhs, rhs, "add");
            } else {
                return LLVMBuildAdd(builder, lhs, rhs, "add");
            }
        case AST_SUB:
            if (isRealType(type) != NULL) {
                return LLVMBuildFSub(builder, lhs, rhs, "sub");
            } else {
                return LLVMBuildSub(builder, lhs, rhs, "sub");
            }
        case AST_MUL:
            if (isRealType(type) != NULL) {
                return LLVMBuildFMul(builder, lhs, rhs, "mul");
            } else {
                return LLVMBuildMul(builder, lhs, rhs, "mul");
            }
        case AST_DIV:
            if (isRealType(type) != NULL) {
                return LLVMBuildFDiv(builder, lhs, rhs, "div");
            } else if (isSignedIntegerType(type) != NULL) {
                return LLVMBuildSDiv(builder, lhs, rhs, "div");
            } else {
                return LLVMBuildUDiv(builder, lhs, rhs, "div");
            }
        case AST_MOD:
            if (isSignedIntegerType(type) != NULL) {
                return LLVMBuildSRem(builder, lhs, rhs, "mod");
            } else {
                return LLVMBuildURem(builder, lhs, rhs, "mod");
            }
        case AST_SHL:
            return LLVMBuildShl(builder, lhs, rhs, "shl");
        case AST_SHR:
            if (isSignedIntegerType(type) != NULL) {
                return LLVMBuildAShr(builder, lhs, rhs, "shr");
            } else {
                return LLVMBuildLShr(builder, lhs, rhs, "shr");
            }
        case AST_BAND:
            return LLVMBuildAnd(builder, lhs, rhs, "and");
        case AST_BOR:
            return LLVMBuildOr(builder, lhs, rhs, "or");
        case AST_BXOR:
            return LLVMBuildXor(builder, lhs, rhs, "xor");
        default:
            UNREACHABLE();
    }
}

static LLVMValueRef buildIntComparison(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, AstNodeKind kind, Type* type) {
    switch (kind) {
        case AST_EQ:
            return LLVMBuildICmp(builder, LLVMIntEQ, lhs, rhs, "eq");
        case AST_NE:
            return LLVMBuildICmp(builder, LLVMIntNE, lhs, rhs, "ne");
        case AST_LE:
            if (isSignedIntegerType(type)) {
                return LLVMBuildICmp(builder, LLVMIntSLE, lhs, rhs, "le");
            } else {
                return LLVMBuildICmp(builder, LLVMIntULE, lhs, rhs, "le");
            }
        case AST_GE:
            if (isSignedIntegerType(type)) {
                return LLVMBuildICmp(builder, LLVMIntSGE, lhs, rhs, "ge");
            } else {
                return LLVMBuildICmp(builder, LLVMIntUGE, lhs, rhs, "ge");
            }
        case AST_LT:
            if (isSignedIntegerType(type)) {
                return LLVMBuildICmp(builder, LLVMIntSLT, lhs, rhs, "lt");
            } else {
                return LLVMBuildICmp(builder, LLVMIntULT, lhs, rhs, "lt");
            }
        case AST_GT:
            if (isSignedIntegerType(type)) {
                return LLVMBuildICmp(builder, LLVMIntSGT, lhs, rhs, "gt");
            } else {
                return LLVMBuildICmp(builder, LLVMIntUGT, lhs, rhs, "gt");
            }
        default:
            UNREACHABLE();
    }
}

static LLVMValueRef buildRealComparison(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, AstNodeKind kind) {
    switch (kind) {
        case AST_EQ:
            return LLVMBuildFCmp(builder, LLVMRealOEQ, lhs, rhs, "eq");
        case AST_NE:
            return LLVMBuildFCmp(builder, LLVMRealONE, lhs, rhs, "ne");
        case AST_LE:
            return LLVMBuildFCmp(builder, LLVMRealOLE, lhs, rhs, "le");
        case AST_GE:
            return LLVMBuildFCmp(builder, LLVMRealOGE, lhs, rhs, "ge");
        case AST_LT:
            return LLVMBuildFCmp(builder, LLVMRealOLT, lhs, rhs, "lt");
        case AST_GT:
            return LLVMBuildFCmp(builder, LLVMRealOGT, lhs, rhs, "gt");
        default:
            UNREACHABLE();
    }
}

static LLVMValueRef buildLlvmIntrinsicCall(
    LlvmCodegenContext* context, LlvmCodegenBodyContext* data, const char* name,
    LLVMValueRef* params, size_t param_count, const char* val_name
) {
    size_t id = LLVMLookupIntrinsicID(name, strlen(name));
    LLVMTypeRef* param_types = ALLOC(LLVMTypeRef, param_count);
    for (size_t i = 0; i < param_count; i++) {
        param_types[i] = LLVMTypeOf(params[i]);
    }
    LLVMValueRef func = LLVMGetIntrinsicDeclaration(data->module, id, param_types, param_count);
    LLVMTypeRef type = LLVMIntrinsicGetType(context->llvm_cxt, id, param_types, param_count);
    FREE(param_types);
    return LLVMBuildCall2(data->builder, type, func, params, param_count, val_name);
}

static LlvmCodegenValue buildFunctionBody(LlvmCodegenContext* context, LlvmCodegenBodyContext* data, AstNode* node) {
    ASSERT(node != NULL);
    switch (node->kind) {
        case AST_ARRAY:
        case AST_ERROR: {
            UNREACHABLE("should not evaluate");
        }
        case AST_LIST: {
            AstList* n = (AstList*)node;
            for (size_t i = 0; i < n->count; i++) {
                buildFunctionBody(context, data, n->nodes[i]);
            }
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_ROOT: {
            AstRoot* n = (AstRoot*)node;
            buildFunctionBody(context, data, (AstNode*)n->nodes);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_BLOCK: {
            AstBlock* n = (AstBlock*)node;
            buildFunctionBody(context, data, (AstNode*)n->nodes);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_VAR: {
            AstVar* n = (AstVar*)node;
            SymbolVariable* var = (SymbolVariable*)n->binding;
            return createLlvmCodegenValue(var->codegen, true);
        }
        case AST_VOID: {
            if (isVoidType(node->res_type) != NULL) {
                LLVMValueRef value = LLVMConstArray(LLVMInt8TypeInContext(context->llvm_cxt), NULL, 0);
                return createLlvmCodegenValue(value, false);
            } else {
                TypeArray* type = isArrayType(node->res_type);
                LLVMValueRef value = LLVMConstArray(generateLlvmType(context, type->base), NULL, 0);
                return createLlvmCodegenValue(value, false);
            }
        }
        case AST_STR: {
            AstStr* n = (AstStr*)node;
            LLVMValueRef value = LLVMConstStringInContext(context->llvm_cxt, n->string.data, n->string.length, false);
            LLVMValueRef global = LLVMAddGlobal(data->module, LLVMTypeOf(value), ".string");
            LLVMSetInitializer(global, value);
            LLVMSetGlobalConstant(global, true);
            LLVMSetLinkage(global, LLVMPrivateLinkage);
            LLVMSetUnnamedAddress(global, LLVMGlobalUnnamedAddr);
            LLVMValueRef ret_value = LLVMBuildPointerCast(data->builder, global, generateLlvmType(context, n->res_type), "cast");
            return createLlvmCodegenValue(ret_value, false);
        }
        case AST_CHAR:
        case AST_INT: {
            AstInt* n = (AstInt*)node;
            LLVMValueRef value = LLVMConstInt(generateLlvmType(context, n->res_type), n->number, isSignedIntegerType(n->res_type) != NULL);
            return createLlvmCodegenValue(value, false);
        }
        case AST_BOOL: {
            AstBool* n = (AstBool*)node;
            LLVMValueRef value = LLVMConstInt(generateLlvmType(context, n->res_type), n->value ? 1 : 0, true);
            return createLlvmCodegenValue(value, false);
        }
        case AST_REAL: {
            AstReal* n = (AstReal*)node;
            LLVMValueRef value = LLVMConstReal(generateLlvmType(context, n->res_type), n->number);
            return createLlvmCodegenValue(value, false);
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
        case AST_BXOR_ASSIGN: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->right);
            LLVMValueRef addrs = getCodegenReference(context, data, n->left);
            LLVMValueRef old_val = LLVMBuildLoad2(data->builder, generateLlvmType(context, n->right->res_type), addrs, "tmp");
            LLVMValueRef new_value = buildBinaryOperation(data->builder, old_val, value, n->right->res_type, n->kind - AST_ASSIGN_OFFSET);
            LLVMBuildStore(data->builder, new_value, addrs);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_ASSIGN: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->right);
            LLVMValueRef addrs = getCodegenReference(context, data, n->left);
            LLVMBuildStore(data->builder, value, addrs);
            return createLlvmCodegenValue(NULL, false);
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
        case AST_BXOR: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef left = getCodegenValue(context, data, n->left);
            LLVMValueRef right = getCodegenValue(context, data, n->right);
            LLVMValueRef value = buildBinaryOperation(data->builder, left, right, n->res_type, n->kind);
            return createLlvmCodegenValue(value, false);
        }
        case AST_OR:
        case AST_AND: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef left = getCodegenValue(context, data, n->left);
            LLVMBasicBlockRef start_block = LLVMGetInsertBlock(data->builder);
            LLVMBasicBlockRef right_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "lazy-right");
            LLVMBasicBlockRef rest_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "lazy-end");
            if (n->kind == AST_AND) {
                LLVMBuildCondBr(data->builder, left, right_block, rest_block);
            } else {
                LLVMBuildCondBr(data->builder, left, rest_block, right_block);
            }
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, right_block);
            LLVMPositionBuilderAtEnd(data->builder, right_block);
            LLVMValueRef right = getCodegenValue(context, data, n->right);
            LLVMBuildBr(data->builder, rest_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            LLVMValueRef value = LLVMBuildPhi(data->builder, generateLlvmType(context, n->res_type), "lazy-res");
            LLVMValueRef incoming_val[2] = { left, right };
            LLVMBasicBlockRef incoming_blk[2] = { start_block, right_block };
            LLVMAddIncoming(value, incoming_val, incoming_blk, 2);
            return createLlvmCodegenValue(value, false);
        }
        case AST_EQ:
        case AST_NE:
        case AST_LE:
        case AST_GE:
        case AST_LT:
        case AST_GT: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef left = getCodegenValue(context, data, n->left);
            LLVMValueRef right = getCodegenValue(context, data, n->right);
            LLVMValueRef value;
            if (isIntegerType(n->left->res_type) != NULL || isBooleanType(n->left->res_type)) {
                value = buildIntComparison(data->builder, left, right, n->kind, n->left->res_type);
            } else if (isPointerType(n->left->res_type)) {
                left = LLVMBuildPtrToInt(data->builder, left, LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data), "tmp");
                right = LLVMBuildPtrToInt(data->builder, right, LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data), "tmp");
                value = buildIntComparison(data->builder, left, right, n->kind, n->left->res_type);
            } else {
                value = buildRealComparison(data->builder, left, right, n->kind);
            }
            return createLlvmCodegenValue(value, false);
        }
        case AST_POS: {
            AstUnary* n = (AstUnary*)node;
            return buildFunctionBody(context, data, n->op);
        }
        case AST_NEG: {
            AstUnary* n = (AstUnary*)node;
            LLVMValueRef op = getCodegenValue(context, data, n->op);
            LLVMValueRef value;
            if (isIntegerType(n->op->res_type) != NULL) {
                value = LLVMBuildNeg(data->builder, op, "neg");
            } else {
                value = LLVMBuildFNeg(data->builder, op, "neg");
            }
            return createLlvmCodegenValue(value, false);
        }
        case AST_NOT: {
            AstUnary* n = (AstUnary*)node;
            LLVMValueRef op = getCodegenValue(context, data, n->op);
            LLVMValueRef value = LLVMBuildNot(data->builder, op, "not");
            return createLlvmCodegenValue(value, false);
        }
        case AST_ADDR: {
            AstUnary* n = (AstUnary*)node;
            LLVMValueRef value = getCodegenReference(context, data, n->op);
            return createLlvmCodegenValue(value, false);
        }
        case AST_DEREF: {
            AstUnary* n = (AstUnary*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->op);
            return createLlvmCodegenValue(value, true);
        }
        case AST_VARDEF: {
            AstVarDef* n = (AstVarDef*)node;
            if (n->val != NULL) {
                LLVMValueRef value = getCodegenValue(context, data, n->val);
                LLVMValueRef addrs = getCodegenReference(context, data, (AstNode*)n->name);
                LLVMBuildStore(data->builder, value, addrs);
            }
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_IF_ELSE: {
            AstIfElse* n = (AstIfElse*)node;
            LLVMBasicBlockRef if_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "if");
            LLVMBasicBlockRef rest_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "endif");
            LLVMBasicBlockRef else_block;
            if (n->else_block != NULL) {
                else_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "else");
            } else {
                else_block = rest_block;
            }
            LLVMValueRef cond = getCodegenValue(context, data, n->condition);
            LLVMBuildCondBr(data->builder, cond, if_block, else_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, if_block);
            LLVMPositionBuilderAtEnd(data->builder, if_block);
            buildFunctionBody(context, data, n->if_block);
            LLVMBuildBr(data->builder, rest_block);
            if (n->else_block != NULL) {
                LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, else_block);
                LLVMPositionBuilderAtEnd(data->builder, else_block);
                buildFunctionBody(context, data, n->else_block);
                LLVMBuildBr(data->builder, rest_block);
            }
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_WHILE: {
            AstWhile* n = (AstWhile*)node;
            LLVMBasicBlockRef cond_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "while-cond");
            LLVMBasicBlockRef while_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "while-body");
            LLVMBasicBlockRef rest_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "endwhile");
            LLVMBuildBr(data->builder, cond_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, cond_block);
            LLVMPositionBuilderAtEnd(data->builder, cond_block);
            LLVMValueRef cond = getCodegenValue(context, data, n->condition);
            LLVMBuildCondBr(data->builder, cond, while_block, rest_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, while_block);
            LLVMPositionBuilderAtEnd(data->builder, while_block);
            buildFunctionBody(context, data, n->block);
            LLVMBuildBr(data->builder, cond_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_CALL: {
            AstCall* n = (AstCall*)node;
            LLVMValueRef func = getCodegenReference(context, data, n->function);
            LLVMValueRef* args = ALLOC(LLVMValueRef, n->arguments->count);
            for (size_t i = 0; i < n->arguments->count; i++) {
                args[i] = getCodegenValue(context, data, n->arguments->nodes[i]);
            }
            LLVMValueRef value = NULL;
            LLVMTypeRef type = generateLlvmType(context, n->function->res_type);
            value = LLVMBuildCall2(
                data->builder, type, func, args,
                n->arguments->count, "call"
            );
            FREE(args);
            return createLlvmCodegenValue(value, false);
        }
        case AST_INDEX: {
            AstBinary* n = (AstBinary*)node;
            if (isPointerType(n->left->res_type) != NULL) {
                LLVMValueRef pointer = getCodegenValue(context, data, n->left);
                LLVMValueRef index = getCodegenValue(context, data, n->right);
                LLVMValueRef value = LLVMBuildGEP2(data->builder, generateLlvmType(context, n->res_type), pointer, &index, 1, "index");
                return createLlvmCodegenValue(value, true);
            } else {
                LlvmCodegenValue array = buildFunctionBody(context, data, n->left);
                LLVMValueRef index = getCodegenValue(context, data, n->right);
                LLVMValueRef indicies[2] = {
                    LLVMConstInt(LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data), 0, false), index
                };
                if (array.is_reference) {
                    LLVMValueRef value = LLVMBuildGEP2(
                        data->builder, generateLlvmType(context, n->left->res_type), array.value, indicies, 2, "index"
                    );
                    return createLlvmCodegenValue(value, true);
                } else {
                    LLVMValueRef stack = buildLlvmIntrinsicCall(context, data, "llvm.stacksave", NULL, 0, "stacksave");
                    LLVMValueRef tmp = LLVMBuildAlloca(
                        data->builder, generateLlvmType(context, n->left->res_type), "tmp"
                    );
                    LLVMBuildStore(data->builder, array.value, tmp);
                    LLVMValueRef value_ref = LLVMBuildGEP2(
                        data->builder, generateLlvmType(context, n->left->res_type), tmp, indicies, 2, "index"
                    );
                    LLVMValueRef value = LLVMBuildLoad2(data->builder, generateLlvmType(context, n->res_type), value_ref, "tmp");
                    buildLlvmIntrinsicCall(context, data, "llvm.stackrestore", &stack, 1, "");
                    return createLlvmCodegenValue(value, false);
                }
            }
        }
        case AST_RETURN: {
            AstReturn* n = (AstReturn*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->value);
            LLVMBuildStore(data->builder, value, data->ret_value);
            LLVMBuildBr(data->builder, data->exit);
            LLVMBasicBlockRef rest_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "dead");
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            return createLlvmCodegenValue(NULL, false);
        }
        case AST_TYPEDEF:
        case AST_FN:
        case AST_ARGDEF:
            return createLlvmCodegenValue(NULL, false);
    }
    UNREACHABLE();
}

static void buildFunctionVariables(LlvmCodegenContext* context, LLVMBuilderRef builder, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_ARRAY: {
                UNREACHABLE("should not evaluate");
            }
            case AST_ERROR:
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
                buildFunctionVariables(context, builder, n->right);
                buildFunctionVariables(context, builder, n->left);
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
                buildFunctionVariables(context, builder, n->left);
                buildFunctionVariables(context, builder, n->right);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                buildFunctionVariables(context, builder, n->left);
                buildFunctionVariables(context, builder, n->right);
                break;
            }
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                buildFunctionVariables(context, builder, n->left);
                buildFunctionVariables(context, builder, n->right);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                buildFunctionVariables(context, builder, n->op);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                buildFunctionVariables(context, builder, n->value);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildFunctionVariables(context, builder, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildFunctionVariables(context, builder, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                buildFunctionVariables(context, builder, (AstNode*)n->nodes);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                SymbolVariable* var = (SymbolVariable*)n->name->binding;
                LLVMValueRef value = LLVMBuildAlloca(builder, generateLlvmType(context, var->type), var->name);
                var->codegen = value;
                buildFunctionVariables(context, builder, n->val);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                buildFunctionVariables(context, builder, n->condition);
                buildFunctionVariables(context, builder, n->if_block);
                buildFunctionVariables(context, builder, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                buildFunctionVariables(context, builder, n->condition);
                buildFunctionVariables(context, builder, n->block);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                buildFunctionVariables(context, builder, n->function);
                buildFunctionVariables(context, builder, (AstNode*)n->arguments);
                break;
            }
            case AST_TYPEDEF:
            case AST_FN:
                break;
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                SymbolVariable* var = (SymbolVariable*)n->name->binding;
                LLVMValueRef value = LLVMBuildAlloca(builder, generateLlvmType(context, var->type), var->name);
                var->codegen = value;
                break;
            }
        }
    }
}

static void buildFunctionBodies(LlvmCodegenContext* context, LLVMModuleRef module, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildFunctionBodies(context, module, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildFunctionBodies(context, module, (AstNode*)n->nodes);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (n->body != NULL) {
                    SymbolVariable* func = (SymbolVariable*)n->name->binding;
                    TypeFunction* type = (TypeFunction*)func->type;
                    LLVMValueRef function = func->codegen;
                    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "entry");
                    LLVMBasicBlockRef body = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "body");
                    LLVMBasicBlockRef exit = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "exit");
                    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context->llvm_cxt);
                    LLVMPositionBuilderAtEnd(builder, entry);
                    LLVMValueRef ret_value = NULL;
                    ret_value = LLVMBuildAlloca(builder, generateLlvmType(context, type->ret_type), "ret_value");
                    buildFunctionVariables(context, builder, (AstNode*)n->arguments);
                    buildFunctionVariables(context, builder, n->body);
                    for (size_t i = 0; i < n->arguments->count; i++) {
                        LLVMValueRef arg_value = LLVMGetParam(function, i);
                        LLVMBuildStore(builder, arg_value, ((AstArgDef*)n->arguments->nodes[i])->name->binding->codegen);
                    }
                    LLVMBuildBr(builder, body);
                    LLVMPositionBuilderAtEnd(builder, body);
                    LlvmCodegenBodyContext data = {
                        .module = module, .ret_value = ret_value,
                        .exit = exit, .builder = builder,
                    };
                    buildFunctionBody(context, &data, n->body);
                    LLVMBuildBr(builder, exit);
                    LLVMPositionBuilderAtEnd(builder, exit);
                    LLVMValueRef ret = LLVMBuildLoad2(builder, generateLlvmType(context, type->ret_type), ret_value, "ret");
                    LLVMBuildRet(builder, ret);
                    LLVMDisposeBuilder(builder);
                }
                break;
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

static void buildFunctionStubs(LlvmCodegenContext* context, LLVMModuleRef module, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildFunctionStubs(context, module, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildFunctionStubs(context, module, (AstNode*)n->nodes);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                SymbolVariable* func = (SymbolVariable*)n->name->binding;
                LLVMValueRef value = LLVMAddFunction(module, n->name->name, generateLlvmType(context, func->type));
                if ((n->flags & AST_FN_FLAG_IMPORT) == 0) {
                    if ((n->flags & AST_FN_FLAG_EXPORT) != 0) {
                        LLVMSetLinkage(value, LLVMExternalLinkage);
                    } else {
                        LLVMSetLinkage(value, LLVMInternalLinkage);
                    }
                }
                func->codegen = value;
                break;
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file) {
    // TODO: emit debug information
    LLVMModuleRef module = LLVMModuleCreateWithNameInContext(cstr(file->original_path), context->llvm_cxt);
    LLVMSetModuleDataLayout(module, context->target_data);
    LLVMSetSourceFileName(module, cstr(file->original_path), file->original_path.length);
    buildFunctionStubs(context, module, file->ast);
    buildFunctionBodies(context, module, file->ast);
    return module;
}

