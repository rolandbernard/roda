
#include <stdbool.h>

#include "codegen/llvm/genmodule.h"

typedef struct {
    LLVMValueRef value;
    bool is_reference;
} LlvmCodegenValue;

static LlvmCodegenValue createLlvmCodegenValue(LLVMValueRef value, bool is_reference) {
    LlvmCodegenValue ret = { .value = value, .is_reference = is_reference };
    return ret;
}

static LlvmCodegenValue buildLlvmModule(LlvmCodegenContext* context, AstNode* node) {
    return createLlvmCodegenValue(NULL, false);
}

LLVMModuleRef generateSingleModule(LlvmCodegenContext* context, File* file) {
    LLVMModuleRef module = LLVMModuleCreateWithName(cstr(file->original_path));
    LLVMSetSourceFileName(module, cstr(file->original_path), file->original_path.length);
    LLVMBuilderRef builder = LLVMCreateBuilder();
    buildLlvmModule(context, file->ast);
    LLVMDisposeBuilder(builder);
    return NULL;
}

