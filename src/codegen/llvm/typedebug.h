#ifndef _LLVM_TYPEDEBUG_H_
#define _LLVM_TYPEDEBUG_H_

#include <llvm-c/Core.h>

#include "compiler/types.h"
#include "codegen/llvm/context.h"

LLVMMetadataRef generateLlvmTypeDebugInfo(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, size_t line);

#endif
