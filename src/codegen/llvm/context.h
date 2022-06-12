#ifndef _LLVM_CONTEXT_H_
#define _LLVM_CONTEXT_H_

#include <llvm-c/Core.h>

#include "compiler/context.h"

typedef struct {
    CompilerContext* cxt;
    LLVMContextRef* llvm_cxt;
} LlvmCodegenContext;

#endif
