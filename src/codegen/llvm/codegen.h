#ifndef _LLVM_CODEGEN_H_
#define _LLVM_CODEGEN_H_

#include "compiler/context.h"

void initLlvmBackend(CompilerContext* context);

void deinitLlvmBackend(CompilerContext* context);

void runCodeGenerationForLlvmIr(CompilerContext* context, ConstPath path);

void runCodeGenerationForLlvmBc(CompilerContext* context, ConstPath path);

void runCodeGenerationForAsm(CompilerContext* context, ConstPath path);

void runCodeGenerationForObj(CompilerContext* context, ConstPath path);

#endif