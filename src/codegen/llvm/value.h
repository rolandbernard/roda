#ifndef _RODA_LLVM_VALUE_H_
#define _RODA_LLVM_VALUE_H_

#include "ast/ast.h"
#include "codegen/llvm/context.h"

typedef struct {
    LLVMValueRef value;
    bool is_reference;
} LlvmCodegenValue;

LlvmCodegenValue createLlvmCodegenValue(LLVMValueRef value, bool is_reference);

LLVMValueRef getCodegenValue(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

LLVMValueRef getCodegenReference(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

#endif
