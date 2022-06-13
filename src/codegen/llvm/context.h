#ifndef _LLVM_CONTEXT_H_
#define _LLVM_CONTEXT_H_

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include "compiler/context.h"

typedef struct {
    CompilerContext* cxt;
    char* error_msg;
    LLVMContextRef llvm_cxt;
    LLVMTargetRef target;
    LLVMTargetDataRef target_data;
    LLVMTargetMachineRef target_machine;
} LlvmCodegenContext;

#endif
