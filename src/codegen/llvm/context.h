#ifndef _RODA_LLVM_CONTEXT_H_
#define _RODA_LLVM_CONTEXT_H_

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#include "compiler/context.h"

typedef struct LlvmCodegenTypeData {
    struct LlvmCodegenTypeData* next;
    LLVMTypeRef type;
    LLVMValueRef value;
    LLVMMetadataRef debug;
    size_t* struct_mapping;
    LLVMBasicBlockRef break_target;
} LlvmCodegenData;

typedef struct {
    CompilerContext* cxt;
    char* error_msg;
    LLVMContextRef llvm_cxt;
    LLVMTargetRef target;
    LLVMTargetDataRef target_data;
    LLVMTargetMachineRef target_machine;
    LlvmCodegenData* codegen_data;
    LLVMTypeRef opaque_type;
} LlvmCodegenContext;

typedef struct {
    LLVMModuleRef module;
    LLVMValueRef ret_value;
    LLVMBasicBlockRef exit;
    LLVMBuilderRef builder;
    LLVMDIBuilderRef debug_bulder;
    LLVMMetadataRef file_metadata;
    LLVMMetadataRef scope_metadata;
} LlvmCodegenModuleContext;

void initLlvmCodegenContext(LlvmCodegenContext* context, CompilerContext* cxt);

void deinitLlvmCodegenContext(LlvmCodegenContext* context);

LlvmCodegenData* createLlvmCodegenData(LlvmCodegenContext* context);

#define CODEGEN(X) ((LlvmCodegenData*)((X)->codegen == NULL ? ((X)->codegen = createLlvmCodegenData(context)) : (X)->codegen))

#endif
