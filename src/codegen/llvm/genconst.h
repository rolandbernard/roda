#ifndef _RODA_LLVM_GENCONST_H_
#define _RODA_LLVM_GENCONST_H_

#include <llvm-c/Core.h>

#include "const/value.h"
#include "codegen/llvm/context.h"

LLVMValueRef generateLlvmConstValue(LlvmCodegenContext* context, ConstValue* value);

#endif
