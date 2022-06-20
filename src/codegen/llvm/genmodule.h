#ifndef _RODA_LLVM_GENMODULE_H_
#define _RODA_LLVM_GENMODULE_H_

#include "llvm-c/Core.h"

#include "ast/ast.h"
#include "compiler/context.h"
#include "codegen/llvm/context.h"

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file);

#endif
