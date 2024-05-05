#ifndef _RODA_LLVM_GENMODULE_H_
#define _RODA_LLVM_GENMODULE_H_

#include "ast/ast.h"
#include "codegen/llvm/context.h"
#include "codegen/llvm/value.h"

LlvmCodegenValue buildLlvmFunctionBody(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file);

#endif
