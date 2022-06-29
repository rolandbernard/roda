
#include "errors/fatalerror.h"
#include "codegen/llvm/genmodule.h"
#include "codegen/llvm/gentype.h"

#include "value.h"

LlvmCodegenValue createLlvmCodegenValue(LLVMValueRef value, bool is_reference) {
    LlvmCodegenValue ret = { .value = value, .is_reference = is_reference };
    return ret;
}

LLVMValueRef getCodegenValue(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    LlvmCodegenValue value = buildLlvmFunctionBody(context, data, node);
    if (value.is_reference) {
        return LLVMBuildLoad2(data->builder, generateLlvmType(context, node->res_type), value.value, "tmp");
    } else {
        return value.value;
    }
}

LLVMValueRef getCodegenReference(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node) {
    LlvmCodegenValue value = buildLlvmFunctionBody(context, data, node);
    ASSERT(value.is_reference);
    return value.value;
}


