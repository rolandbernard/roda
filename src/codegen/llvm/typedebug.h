#ifndef _RODA_LLVM_TYPEDEBUG_H_
#define _RODA_LLVM_TYPEDEBUG_H_

#include <llvm-c/Core.h>

#include "types/types.h"
#include "codegen/llvm/context.h"

LLVMMetadataRef generateLlvmTypeDebugInfo(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, size_t line);

#endif
