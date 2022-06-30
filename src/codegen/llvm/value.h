#ifndef _RODA_LLVM_VALUE_H_
#define _RODA_LLVM_VALUE_H_

#include "ast/ast.h"
#include "codegen/llvm/context.h"

typedef struct {
    LLVMValueRef value;
    bool is_reference;
} LlvmCodegenValue;

LlvmCodegenValue createLlvmCodegenValue(LLVMValueRef value, bool is_reference);

LlvmCodegenValue createLlvmCodegenVoidValue(LlvmCodegenContext* context);

LlvmCodegenValue toNonReferenceCodegenValue(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, LlvmCodegenValue value);

LLVMValueRef extractCodegenValue(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, LlvmCodegenValue value);

LLVMValueRef getCodegenValue(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

LLVMValueRef getCodegenReference(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

void buildLlvmStore(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, LLVMValueRef value, LLVMValueRef ptr);

#endif
