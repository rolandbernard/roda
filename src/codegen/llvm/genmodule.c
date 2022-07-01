
#include <llvm-c/DebugInfo.h>
#include <stdbool.h>
#include <string.h>

#include "codegen/llvm/gentype.h"
#include "codegen/llvm/typedebug.h"
#include "errors/fatalerror.h"
#include "rodac/version.h"
#include "util/alloc.h"

#include "codegen/llvm/genmodule.h"
#include "llvm-c/Core.h"

static LLVMValueRef buildBinaryOperation(LLVMBuilderRef builder, LLVMValueRef lhs, LLVMValueRef rhs, Type* type, AstNodeKind kind) {
    switch (kind) {
        case AST_ADD:
            if (isRealType(type)) {
                return LLVMBuildFAdd(builder, lhs, rhs, "add");
            } else {
                return LLVMBuildAdd(builder, lhs, rhs, "add");
            }
        case AST_SUB:
            if (isRealType(type)) {
                return LLVMBuildFSub(builder, lhs, rhs, "sub");
            } else {
                return LLVMBuildSub(builder, lhs, rhs, "sub");
            }
        case AST_MUL:
            if (isRealType(type)) {
                return LLVMBuildFMul(builder, lhs, rhs, "mul");
            } else {
                return LLVMBuildMul(builder, lhs, rhs, "mul");
            }
        case AST_DIV:
            if (isRealType(type)) {
                return LLVMBuildFDiv(builder, lhs, rhs, "div");
            } else if (isSignedIntegerType(type)) {
                return LLVMBuildSDiv(builder, lhs, rhs, "div");
            } else {
                return LLVMBuildUDiv(builder, lhs, rhs, "div");
            }
        case AST_MOD:
            if (isSignedIntegerType(type)) {
                return LLVMBuildSRem(builder, lhs, rhs, "mod");
            } else {
                return LLVMBuildURem(builder, lhs, rhs, "mod");
            }
        case AST_SHL:
            return LLVMBuildShl(builder, lhs, rhs, "shl");
        case AST_SHR:
            if (isSignedIntegerType(type)) {
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
    LlvmCodegenContext* context, LlvmCodegenModuleContext* data, const char* name,
    LLVMValueRef* params, size_t param_count, const char* val_name, bool overloaded
) {
    size_t id = LLVMLookupIntrinsicID(name, strlen(name));
    LLVMValueRef func;
    LLVMTypeRef type;
    if (overloaded) {
        LLVMTypeRef* param_types = ALLOC(LLVMTypeRef, param_count);
        for (size_t i = 0; i < param_count; i++) {
            param_types[i] = LLVMTypeOf(params[i]);
        }
        func = LLVMGetIntrinsicDeclaration(data->module, id, param_types, param_count);
        type = LLVMIntrinsicGetType(context->llvm_cxt, id, param_types, param_count);
        FREE(param_types);
    } else {
        func = LLVMGetIntrinsicDeclaration(data->module, id, NULL, 0);
        type = LLVMIntrinsicGetType(context->llvm_cxt, id, NULL, 0);
    }
    return LLVMBuildCall2(data->builder, type, func, params, param_count, val_name);
}

LlvmCodegenValue buildLlvmFunctionBody(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    ASSERT(node != NULL);
    if (context->cxt->settings.emit_debug) {
        LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
            context->llvm_cxt, node->location.begin.line + 1, node->location.begin.column + 1,
            data->scope_metadata, NULL
        );
        LLVMSetCurrentDebugLocation2(data->builder, loc);
    }
    switch (node->kind) {
        case AST_STRUCT_TYPE:
        case AST_ARRAY:
        case AST_FN_TYPE:
        case AST_ERROR: {
            UNREACHABLE("should not evaluate");
        }
        case AST_STRUCT_LIT: {
            AstList* n = (AstList*)node;
            LLVMTypeRef llvm_type = generateLlvmType(context, n->res_type);
            TypeStruct* type = (TypeStruct*)getTypeOfKind(n->res_type, TYPE_STRUCT);
            LLVMValueRef* fields = ALLOC(LLVMValueRef, type->count);
            bool all_const = true;
            for (size_t i = 0; i < type->count; i++) {
                AstStructField* field = (AstStructField*)n->nodes[i];
                size_t idx = lookupIndexOfStructField(type, field->name->name);
                idx = CODEGEN(type)->struct_mapping[idx];
                fields[idx] = getCodegenValue(context, data, field->field_value);
                if (!LLVMIsConstant(fields[idx])) {
                    all_const = false;
                }
            }
            LLVMValueRef value = NULL;
            if (all_const) {
                value = LLVMConstStructInContext(context->llvm_cxt, fields, type->count, false);
                value = LLVMConstBitCast(value, llvm_type);
            } else {
                LLVMValueRef stack = buildLlvmIntrinsicCall(context, data, "llvm.stacksave", NULL, 0, "stacksave", false);
                LLVMValueRef tmp = LLVMBuildAlloca(data->builder, llvm_type, "tmp");
                for (size_t i = 0; i < type->count; i++) {
                    LLVMValueRef value_ref = LLVMBuildStructGEP2(
                        data->builder, generateLlvmType(context, n->res_type), tmp, i, "index"
                    );
                    buildLlvmStore(context, data, type->types[i], fields[i], value_ref);
                }
                value = LLVMBuildLoad2(data->builder, generateLlvmType(context, n->res_type), tmp, "tmp");
                buildLlvmIntrinsicCall(context, data, "llvm.stackrestore", &stack, 1, "", false);
            }
            FREE(fields);
            return createLlvmCodegenValue(value, false);
        }
        case AST_ARRAY_LIT: {
            AstList* n = (AstList*)node;
            LLVMTypeRef llvm_type = generateLlvmType(context, n->res_type);
            TypeArray* type = (TypeArray*)getTypeOfKind(node->res_type, TYPE_ARRAY);
            LLVMValueRef* values = ALLOC(LLVMValueRef, type->size);
            bool all_const = true;
            for (size_t i = 0; i < type->size; i++) {
                values[i] = getCodegenValue(context, data, n->nodes[i]);
                if (!LLVMIsConstant(values[i])) {
                    all_const = false;
                }
            }
            LLVMValueRef value = NULL;
            if (all_const) {
                LLVMTypeRef elem_type = generateLlvmType(context, type->base);
                value = LLVMConstArray(elem_type, values, type->size);
                value = LLVMConstBitCast(value, llvm_type);
            } else {
                LLVMValueRef stack = buildLlvmIntrinsicCall(context, data, "llvm.stacksave", NULL, 0, "stacksave", false);
                LLVMValueRef tmp = LLVMBuildAlloca(data->builder, generateLlvmType(context, n->res_type), "tmp");
                LLVMTypeRef idx_type = LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data);
                for (size_t i = 0; i < type->size; i++) {
                    LLVMValueRef indicies[2] = { LLVMConstInt(idx_type, 0, false), LLVMConstInt(idx_type, i, false) };
                    LLVMValueRef value_ref = LLVMBuildGEP2(
                        data->builder, generateLlvmType(context, n->res_type), tmp, indicies, 2, "index"
                    );
                    buildLlvmStore(context, data, n->nodes[i]->res_type, values[i], value_ref);
                }
                value = LLVMBuildLoad2(data->builder, generateLlvmType(context, n->res_type), tmp, "tmp");
                buildLlvmIntrinsicCall(context, data, "llvm.stackrestore", &stack, 1, "", false);
            }
            FREE(values);
            return createLlvmCodegenValue(value, false);
        }
        case AST_LIST: {
            AstList* n = (AstList*)node;
            for (size_t i = 0; i < n->count; i++) {
                buildLlvmFunctionBody(context, data, n->nodes[i]);
            }
            return createLlvmCodegenVoidValue(context);
        }
        case AST_ROOT: {
            AstRoot* n = (AstRoot*)node;
            buildLlvmFunctionBody(context, data, (AstNode*)n->nodes);
            return createLlvmCodegenVoidValue(context);
        }
        case AST_BLOCK: {
            AstBlock* n = (AstBlock*)node;
            LLVMMetadataRef old_scope = data->scope_metadata;
            data->scope_metadata = CODEGEN(n)->debug;
            buildLlvmFunctionBody(context, data, (AstNode*)n->nodes);
            data->scope_metadata = old_scope;
            return createLlvmCodegenVoidValue(context);
        }
        case AST_BLOCK_EXPR: {
            AstBlock* n = (AstBlock*)node;
            LLVMMetadataRef old_scope = data->scope_metadata;
            data->scope_metadata = CODEGEN(n)->debug;
            LlvmCodegenValue value = createLlvmCodegenValue(NULL, false);
            for (size_t i = 0; i < n->nodes->count; i++) {
                value = buildLlvmFunctionBody(context, data, n->nodes->nodes[i]);
            }
            data->scope_metadata = old_scope;
            return value;
        }
        case AST_VAR: {
            AstVar* n = (AstVar*)node;
            SymbolVariable* var = (SymbolVariable*)n->binding;
            return createLlvmCodegenValue(CODEGEN(var)->value, true);
        }
        case AST_VOID: {
            LLVMValueRef value = LLVMGetUndef(generateLlvmType(context, node->res_type));
            return createLlvmCodegenValue(value, false);
        }
        case AST_STR: {
            AstStr* n = (AstStr*)node;
            LLVMValueRef value = LLVMConstStringInContext(context->llvm_cxt, n->string.data, n->string.length, false);
            LLVMValueRef global = LLVMAddGlobal(data->module, LLVMTypeOf(value), ".str");
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
            LLVMValueRef value = LLVMConstInt(generateLlvmType(context, n->res_type), n->number, isSignedIntegerType(n->res_type));
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
            buildLlvmStore(context, data, n->right->res_type, new_value, addrs);
            return createLlvmCodegenVoidValue(context);
        }
        case AST_ASSIGN: {
            AstBinary* n = (AstBinary*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->right);
            LLVMValueRef addrs = getCodegenReference(context, data, n->left);
            buildLlvmStore(context, data, n->right->res_type, value, addrs);
            return createLlvmCodegenVoidValue(context);
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
            LLVMBasicBlockRef other_block = LLVMGetInsertBlock(data->builder);
            LLVMBuildBr(data->builder, rest_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            LLVMValueRef value = LLVMBuildPhi(data->builder, generateLlvmType(context, n->res_type), "lazy-res");
            LLVMValueRef incoming_val[2] = { left, right };
            LLVMBasicBlockRef incoming_blk[2] = { start_block, other_block };
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
            LLVMValueRef value = NULL;
            if (isIntegerType(n->left->res_type) || isBooleanType(n->left->res_type)) {
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
            return buildLlvmFunctionBody(context, data, n->op);
        }
        case AST_NEG: {
            AstUnary* n = (AstUnary*)node;
            LLVMValueRef op = getCodegenValue(context, data, n->op);
            LLVMValueRef value;
            if (isIntegerType(n->op->res_type)) {
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
        case AST_SIZEOF: {
            AstUnary* n = (AstUnary*)node;
            LLVMTypeRef type = generateLlvmType(context, n->op->res_type);
            LLVMValueRef value = LLVMConstInt(generateLlvmType(context, n->res_type), LLVMABISizeOfType(context->target_data, type), false);
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
                buildLlvmStore(context, data, n->val->res_type, value, addrs);
            }
            return createLlvmCodegenVoidValue(context);
        }
        case AST_IF_ELSE_EXPR:
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
            LLVMValueRef if_value = getCodegenValue(context, data, n->if_block);
            LLVMValueRef else_value = NULL;
            LLVMBuildBr(data->builder, rest_block);
            if (n->else_block != NULL) {
                LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, else_block);
                LLVMPositionBuilderAtEnd(data->builder, else_block);
                else_value = getCodegenValue(context, data, n->else_block);
                LLVMBuildBr(data->builder, rest_block);
            }
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            if (node->kind == AST_IF_ELSE_EXPR) {
                LLVMValueRef value = LLVMBuildPhi(data->builder, generateLlvmType(context, n->res_type), "if-else");
                LLVMValueRef incoming_val[2] = { if_value, else_value };
                LLVMBasicBlockRef incoming_blk[2] = { if_block, else_block };
                LLVMAddIncoming(value, incoming_val, incoming_blk, 2);
                return createLlvmCodegenValue(value, false);
            } else {
                return createLlvmCodegenVoidValue(context);
            }
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
            buildLlvmFunctionBody(context, data, n->block);
            LLVMBuildBr(data->builder, cond_block);
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            return createLlvmCodegenVoidValue(context);
        }
        case AST_CALL: {
            AstCall* n = (AstCall*)node;
            LLVMValueRef func = getCodegenReference(context, data, n->function);
            LLVMValueRef* args = ALLOC(LLVMValueRef, n->arguments->count);
            for (size_t i = 0; i < n->arguments->count; i++) {
                args[i] = getCodegenValue(context, data, n->arguments->nodes[i]);
            }
            LLVMTypeRef type = generateLlvmType(context, n->function->res_type);
            LLVMValueRef value = LLVMBuildCall2(
                data->builder, type, func, args,
                n->arguments->count, "call"
            );
            FREE(args);
            return createLlvmCodegenValue(value, false);
        }
        case AST_AS: {
            AstBinary* n = (AstBinary*)node;
            if (compareStructuralTypes(n->left->res_type, n->res_type)) {
                // No type change
                return buildLlvmFunctionBody(context, data, n->left);
            } else {
                LLVMValueRef op = getCodegenValue(context, data, n->left);
                LLVMTypeRef type = generateLlvmType(context, n->res_type);
                LLVMValueRef value = NULL;
                if (isIntegerType(n->res_type)) {
                    if (isIntegerType(n->left->res_type)) {
                        bool sign = isSignedIntegerType(n->left->res_type);
                        value = LLVMBuildIntCast2(data->builder, op, type, sign, "cast");
                    } else if (isRealType(n->left->res_type)) {
                        if (isSignedIntegerType(n->res_type)) {
                            value = LLVMBuildFPToSI(data->builder, op, type, "cast");
                        } else {
                            value = LLVMBuildFPToUI(data->builder, op, type, "cast");
                        }
                    } else if (isPointerType(n->left->res_type)) {
                        value = LLVMBuildPtrToInt(data->builder, op, type, "cast");
                    }
                } else if (isRealType(n->res_type)) {
                    if (isRealType(n->left->res_type)) {
                        value = LLVMBuildFPCast(data->builder, op, type, "cast");
                    } else if (isIntegerType(n->left->res_type)) {
                        if (isSignedIntegerType(n->left->res_type)) {
                            value = LLVMBuildSIToFP(data->builder, op, type, "cast");
                        } else {
                            value = LLVMBuildUIToFP(data->builder, op, type, "cast");
                        }
                    }
                } else if (isPointerType(n->res_type)) {
                    if (isPointerType(n->left->res_type)) {
                        value = LLVMBuildPointerCast(data->builder, op, type, "cast");
                    } else if (isIntegerType(n->left->res_type)) {
                        value = LLVMBuildIntToPtr(data->builder, op, type, "cast");
                    }
                }
                return createLlvmCodegenValue(value, false);
            }
        }
        case AST_INDEX: {
            AstBinary* n = (AstBinary*)node;
            if (isPointerType(n->left->res_type)) {
                LLVMValueRef pointer = getCodegenValue(context, data, n->left);
                LLVMValueRef index = getCodegenValue(context, data, n->right);
                LLVMValueRef value = LLVMBuildGEP2(data->builder, generateLlvmType(context, n->res_type), pointer, &index, 1, "index");
                return createLlvmCodegenValue(value, true);
            } else {
                LlvmCodegenValue array = buildLlvmFunctionBody(context, data, n->left);
                LLVMValueRef index = getCodegenValue(context, data, n->right);
                LLVMValueRef indicies[2] = {
                    LLVMConstInt(LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data), 0, false), index
                };
                if (array.is_reference) {
                    LLVMValueRef value = LLVMBuildGEP2(
                        data->builder, generateLlvmType(context, n->left->res_type), array.value, indicies, 2, "index"
                    );
                    return createLlvmCodegenValue(value, true);
                } else if (LLVMIsConstant(index)) {
                    size_t idx = LLVMConstIntGetZExtValue(index);
                    LLVMValueRef value = LLVMBuildExtractValue(data->builder, array.value, idx, "index");
                    return createLlvmCodegenValue(value, false);
                } else {
                    LLVMValueRef stack = buildLlvmIntrinsicCall(context, data, "llvm.stacksave", NULL, 0, "stacksave", false);
                    LLVMValueRef tmp = LLVMBuildAlloca(
                        data->builder, generateLlvmType(context, n->left->res_type), "tmp"
                    );
                    buildLlvmStore(context, data, n->left->res_type, array.value, tmp);
                    LLVMValueRef value_ref = LLVMBuildGEP2(
                        data->builder, generateLlvmType(context, n->left->res_type), tmp, indicies, 2, "index"
                    );
                    LLVMValueRef value = LLVMBuildLoad2(data->builder, generateLlvmType(context, n->res_type), value_ref, "tmp");
                    buildLlvmIntrinsicCall(context, data, "llvm.stackrestore", &stack, 1, "", false);
                    return createLlvmCodegenValue(value, false);
                }
            }
        }
        case AST_RETURN: {
            AstReturn* n = (AstReturn*)node;
            LLVMValueRef value = getCodegenValue(context, data, n->value);
            buildLlvmStore(context, data, n->value->res_type, value, data->ret_value);
            LLVMBuildBr(data->builder, data->exit);
            LLVMBasicBlockRef rest_block = LLVMCreateBasicBlockInContext(context->llvm_cxt, "dead");
            LLVMInsertExistingBasicBlockAfterInsertBlock(data->builder, rest_block);
            LLVMPositionBuilderAtEnd(data->builder, rest_block);
            return createLlvmCodegenVoidValue(context);
        }
        case AST_STRUCT_INDEX: {
            AstStructIndex* n = (AstStructIndex*)node;
            TypeStruct* type = (TypeStruct*)getTypeOfKind(n->strct->res_type, TYPE_STRUCT);
            LLVMTypeRef strct_type = generateLlvmType(context, n->strct->res_type);
            LlvmCodegenValue strct = buildLlvmFunctionBody(context, data, n->strct);
            size_t idx = lookupIndexOfStructField(type, n->field->name);
            idx = CODEGEN(type)->struct_mapping[idx];
            if (strct.is_reference) {
                LLVMValueRef value = LLVMBuildStructGEP2(
                    data->builder, strct_type, strct.value, idx, "index"
                );
                return createLlvmCodegenValue(value, true);
            } else {
                LLVMValueRef value = LLVMBuildExtractValue(data->builder, strct.value, idx, "index");
                return createLlvmCodegenValue(value, false);
            }
        }
        case AST_TYPEDEF:
        case AST_FN:
        case AST_ARGDEF:
            return createLlvmCodegenVoidValue(context);
    }
    UNREACHABLE();
}

static void buildFunctionVariables(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_FN_TYPE:
            case AST_STRUCT_TYPE:
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
                buildFunctionVariables(context, data, n->right);
                buildFunctionVariables(context, data, n->left);
                break;
            }
            case AST_AS: {
                AstBinary* n = (AstBinary*)node;
                buildFunctionVariables(context, data, n->left);
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
                buildFunctionVariables(context, data, n->left);
                buildFunctionVariables(context, data, n->right);
                break;
            }
            case AST_OR:
            case AST_AND: {
                AstBinary* n = (AstBinary*)node;
                buildFunctionVariables(context, data, n->left);
                buildFunctionVariables(context, data, n->right);
                break;
            }
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT: {
                AstBinary* n = (AstBinary*)node;
                buildFunctionVariables(context, data, n->left);
                buildFunctionVariables(context, data, n->right);
                break;
            }
            case AST_SIZEOF:
                break;
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                buildFunctionVariables(context, data, n->op);
                break;
            }
            case AST_RETURN: {
                AstReturn* n = (AstReturn*)node;
                buildFunctionVariables(context, data, n->value);
                break;
            }
            case AST_STRUCT_LIT: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    AstStructField* field = (AstStructField*)n->nodes[i];
                    buildFunctionVariables(context, data, field->field_value);
                }
                break;
            }
            case AST_ARRAY_LIT:
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildFunctionVariables(context, data, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildFunctionVariables(context, data, (AstNode*)n->nodes);
                break;
            }
            case AST_BLOCK_EXPR:
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                LLVMMetadataRef old_scope = data->scope_metadata;
                if (context->cxt->settings.emit_debug) {
                    data->scope_metadata = LLVMDIBuilderCreateLexicalBlock(
                        data->debug_bulder, data->scope_metadata, data->file_metadata,
                        n->location.begin.line + 1, n->location.begin.column + 1
                    );
                    CODEGEN(n)->debug = data->scope_metadata;
                }
                buildFunctionVariables(context, data, (AstNode*)n->nodes);
                if (context->cxt->settings.emit_debug) {
                    data->scope_metadata = old_scope;
                }
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                SymbolVariable* var = (SymbolVariable*)n->name->binding;
                LLVMTypeRef type = generateLlvmType(context, var->type);
                LLVMValueRef value = LLVMBuildAlloca(data->builder, type, var->name);
                CODEGEN(var)->value = value;
                if (context->cxt->settings.emit_debug) {
                    LLVMMetadataRef type_meta = generateLlvmTypeDebugInfo(context, data, var->type, n->location.begin.line + 1);
                    LLVMMetadataRef param_meta = LLVMDIBuilderCreateAutoVariable(
                        data->debug_bulder, data->scope_metadata, var->name, strlen(var->name),
                        data->file_metadata, n->location.begin.line + 1, type_meta, false, 0,
                        8 * LLVMABIAlignmentOfType(context->target_data, type)
                    );
                    CODEGEN(var)->debug = param_meta;
                    LLVMMetadataRef param_loc = LLVMDIBuilderCreateDebugLocation(
                        context->llvm_cxt, n->location.begin.line + 1,
                        n->location.begin.column + 1, data->scope_metadata, NULL
                    );
                    LLVMDIBuilderInsertDeclareAtEnd(
                        data->debug_bulder, value, param_meta,
                        LLVMDIBuilderCreateExpression(data->debug_bulder, NULL, 0), param_loc,
                        LLVMGetInsertBlock(data->builder)
                    );
                }
                buildFunctionVariables(context, data, n->val);
                break;
            }
            case AST_IF_ELSE_EXPR:
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                buildFunctionVariables(context, data, n->condition);
                buildFunctionVariables(context, data, n->if_block);
                buildFunctionVariables(context, data, n->else_block);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                buildFunctionVariables(context, data, n->condition);
                buildFunctionVariables(context, data, n->block);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                buildFunctionVariables(context, data, n->function);
                buildFunctionVariables(context, data, (AstNode*)n->arguments);
                break;
            }
            case AST_TYPEDEF:
            case AST_FN:
                break;
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                SymbolVariable* var = (SymbolVariable*)n->name->binding;
                LLVMValueRef value = LLVMBuildAlloca(data->builder, generateLlvmType(context, var->type), var->name);
                CODEGEN(var)->value = value;
                if (context->cxt->settings.emit_debug) {
                    LLVMMetadataRef type_meta = generateLlvmTypeDebugInfo(context, data, var->type, n->location.begin.line + 1);
                    LLVMMetadataRef param_meta = LLVMDIBuilderCreateParameterVariable(
                        data->debug_bulder, data->scope_metadata, var->name, strlen(var->name),
                        n->parent_idx, data->file_metadata, n->location.begin.line + 1, type_meta,
                        false, 0
                    );
                    CODEGEN(var)->debug = param_meta;
                    LLVMMetadataRef param_loc = LLVMDIBuilderCreateDebugLocation(
                        context->llvm_cxt, n->location.begin.line + 1,
                        n->location.begin.column + 1, data->scope_metadata, NULL
                    );
                    LLVMDIBuilderInsertDeclareAtEnd(
                        data->debug_bulder, value, param_meta,
                        LLVMDIBuilderCreateExpression(data->debug_bulder, NULL, 0), param_loc,
                        LLVMGetInsertBlock(data->builder)
                    );
                }
                break;
            }
            case AST_STRUCT_INDEX: {
                AstStructIndex* n = (AstStructIndex*)node;
                buildFunctionVariables(context, data, n->strct);
                break;
            }
        }
    }
}

static void buildFunctionBodies(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildFunctionBodies(context, data, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildFunctionBodies(context, data, (AstNode*)n->nodes);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                if (n->body != NULL) {
                    SymbolVariable* func = (SymbolVariable*)n->name->binding;
                    TypeFunction* type = (TypeFunction*)func->type;
                    LLVMMetadataRef old_scope = data->scope_metadata;
                    data->scope_metadata  = CODEGEN(func)->debug;
                    LLVMValueRef function = CODEGEN(func)->value;
                    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "entry");
                    LLVMBasicBlockRef body = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "body");
                    data->exit = LLVMAppendBasicBlockInContext(context->llvm_cxt, function, "exit");
                    LLVMPositionBuilderAtEnd(data->builder, entry);
                    data->ret_value = LLVMBuildAlloca(data->builder, generateLlvmType(context, type->ret_type), "ret_value");
                    buildFunctionVariables(context, data, (AstNode*)n->arguments);
                    buildFunctionVariables(context, data, n->body);
                    for (size_t i = 0; i < n->arguments->count; i++) {
                        AstArgDef* def = (AstArgDef*)n->arguments->nodes[i];
                        LLVMValueRef arg_value = LLVMGetParam(function, i);
                        buildLlvmStore(context, data, def->name->res_type, arg_value, CODEGEN(def->name->binding)->value);
                    }
                    LLVMBuildBr(data->builder, body);
                    LLVMPositionBuilderAtEnd(data->builder, body);
                    buildLlvmFunctionBody(context, data, n->body);
                    if (context->cxt->settings.emit_debug) {
                        LLVMSetCurrentDebugLocation2(data->builder, NULL);
                    }
                    LLVMBuildBr(data->builder, data->exit);
                    LLVMPositionBuilderAtEnd(data->builder, data->exit);
                    LLVMValueRef ret = LLVMBuildLoad2(
                        data->builder, generateLlvmType(context, type->ret_type), data->ret_value, "ret"
                    );
                    LLVMBuildRet(data->builder, ret);
                    data->scope_metadata = old_scope;
                }
                break;
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

static void buildGlobalsAndFunctionStubs(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    if (node != NULL) {
        switch (node->kind) {
            case AST_LIST: {
                AstList* n = (AstList*)node;
                for (size_t i = 0; i < n->count; i++) {
                    buildGlobalsAndFunctionStubs(context, data, n->nodes[i]);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                buildGlobalsAndFunctionStubs(context, data, (AstNode*)n->nodes);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                SymbolVariable* func = (SymbolVariable*)n->name->binding;
                LLVMValueRef value = LLVMAddFunction(data->module, func->name, generateLlvmType(context, func->type));
                bool local = false;
                if ((n->flags & AST_FN_FLAG_EXTERN) == 0) {
                    if ((n->flags & AST_FN_FLAG_PUBLIC) != 0) {
                        LLVMSetLinkage(value, LLVMExternalLinkage);
                    } else {
                        LLVMSetLinkage(value, LLVMInternalLinkage);
                        local = true;
                    }
                }
                if (context->cxt->settings.emit_debug && n->body != NULL) {
                    LLVMMetadataRef type = generateLlvmTypeDebugInfo(context, data, func->type, n->location.begin.line + 1);
                    LLVMMetadataRef debug_func = LLVMDIBuilderCreateFunction(
                        data->debug_bulder, data->scope_metadata, func->name, strlen(func->name),
                        func->name, strlen(func->name), data->file_metadata,
                        n->location.begin.line + 1, type, local, true,
                        n->location.begin.line + 1, 0,
                        context->cxt->settings.opt_level != COMPILER_OPT_NONE
                            && context->cxt->settings.opt_level != COMPILER_OPT_DEFAULT
                    );
                    CODEGEN(func)->debug = debug_func;
                    LLVMSetSubprogram(value, debug_func);
                }
                CODEGEN(func)->value = value;
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                SymbolVariable* var = (SymbolVariable*)n->name->binding;
                LLVMTypeRef type = generateLlvmType(context, var->type);
                LLVMValueRef value = LLVMAddGlobal(data->module, type, var->name);
                CODEGEN(var)->value = value;
                LLVMSetExternallyInitialized(value, (n->flags & AST_VAR_FLAG_EXTERN) != 0);
                bool local = false;
                if ((n->flags & AST_VAR_FLAG_EXTERN) == 0) {
                    LLVMSetInitializer(value, LLVMConstNull(type));
                    if ((n->flags & AST_VAR_FLAG_PUBLIC) != 0) {
                        LLVMSetLinkage(value, LLVMExternalLinkage);
                    } else {
                        LLVMSetLinkage(value, LLVMInternalLinkage);
                        local = true;
                    }
                }
                if (context->cxt->settings.emit_debug) {
                    LLVMMetadataRef type_meta = generateLlvmTypeDebugInfo(context, data, var->type, n->location.begin.line + 1);
                    LLVMMetadataRef var_meta = LLVMDIBuilderCreateGlobalVariableExpression(
                        data->debug_bulder, data->scope_metadata, var->name, strlen(var->name),
                        var->name, strlen(var->name), data->file_metadata,
                        node->location.begin.line + 1, type_meta, local,
                        LLVMDIBuilderCreateExpression(data->debug_bulder, NULL, 0), NULL,
                        LLVMABIAlignmentOfType(context->target_data, type)
                    );
                    CODEGEN(var)->debug = var_meta;
                    LLVMGlobalSetMetadata(value, 0, var_meta);
                }
            }
            default:
                // We only want to consider the global scope
                break;
        }
    }
}

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file) {
    LlvmCodegenModuleContext data;
    data.module = LLVMModuleCreateWithNameInContext(cstr(file->original_path), context->llvm_cxt);
    LLVMSetModuleDataLayout(data.module, context->target_data);
    LLVMSetSourceFileName(data.module, cstr(file->original_path), file->original_path.length);
    data.builder = LLVMCreateBuilderInContext(context->llvm_cxt);
    if (context->cxt->settings.emit_debug) {
        data.debug_bulder = LLVMCreateDIBuilder(data.module);
        data.file_metadata = LLVMDIBuilderCreateFile(
            data.debug_bulder, file->name.data, file->name.length,
            file->directory.data, file->directory.length
        );
        data.scope_metadata = data.file_metadata;
#if LLVM_VERSION_MAJOR >= 11
        LLVMDIBuilderCreateCompileUnit(
            data.debug_bulder, LLVMDWARFSourceLanguageC, data.file_metadata, PROGRAM_NAME " v" VERSION_STRING_BUILD,
            strlen(PROGRAM_NAME " v" VERSION_STRING_BUILD),
            context->cxt->settings.opt_level != COMPILER_OPT_NONE
                && context->cxt->settings.opt_level != COMPILER_OPT_DEFAULT,
            NULL, 0, 0, NULL, 0, LLVMDWARFEmissionFull, 0, false, false, NULL, 0, NULL, 0
        );
#else
        LLVMDIBuilderCreateCompileUnit(
            data.debug_bulder, LLVMDWARFSourceLanguageC, data.file_metadata, PROGRAM_NAME " v" VERSION_STRING_BUILD,
            strlen(PROGRAM_NAME " v" VERSION_STRING_BUILD),
            context->cxt->settings.opt_level != COMPILER_OPT_NONE
                && context->cxt->settings.opt_level != COMPILER_OPT_DEFAULT,
            NULL, 0, 0, NULL, 0, LLVMDWARFEmissionFull, 0, false, false
        );
#endif
        LLVMAddModuleFlag(
            data.module, LLVMModuleFlagBehaviorWarning, "Debug Info Version", 18,
            LLVMValueAsMetadata(LLVMConstInt(LLVMIntType(32), LLVMDebugMetadataVersion(), 0))
        );
    }
    buildGlobalsAndFunctionStubs(context, &data, file->ast);
    buildFunctionBodies(context, &data, file->ast);
    if (context->cxt->settings.emit_debug) {
        LLVMDIBuilderFinalize(data.debug_bulder);
        LLVMDisposeDIBuilder(data.debug_bulder);
    }
    LLVMDisposeBuilder(data.builder);
    return data.module;
}

