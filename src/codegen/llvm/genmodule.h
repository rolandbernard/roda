#ifndef _RODA_LLVM_GENMODULE_H_
#define _RODA_LLVM_GENMODULE_H_

#include "llvm-c/Core.h"

#include "ast/ast.h"
#include "compiler/context.h"
#include "codegen/llvm/context.h"
#include "codegen/llvm/value.h"

LlvmCodegenValue buildLlvmFunctionBody(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, AstNode* node);

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file);

#endif
