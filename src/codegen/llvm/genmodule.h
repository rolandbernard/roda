#ifndef _LLVM_GENMODULE_H_
#define _LLVM_GENMODULE_H_

#include "llvm-c/Core.h"

#include "ast/ast.h"
#include "compiler/context.h"

LLVMModuleRef generateSingleModule(CompilerContext* context, File* file);

#endif
