#ifndef _RODA_LLVM_GENTYPE_H_
#define _RODA_LLVM_GENTYPE_H_

#include <llvm-c/Core.h>

#include "types/types.h"
#include "codegen/llvm/context.h"

LLVMTypeRef generateLlvmType(LlvmCodegenContext* context, Type* type);

#endif
