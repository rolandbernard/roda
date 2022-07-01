
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"

#include "codegen/llvm/context.h"

static LLVMCodeGenOptLevel getLlvmCodeGenOptLevel(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case COMPILER_OPT_DEFAULT:
            return context->cxt->settings.emit_debug
                ? LLVMCodeGenLevelNone : LLVMCodeGenLevelDefault;
        case COMPILER_OPT_NONE:
            return LLVMCodeGenLevelNone;
        case COMPILER_OPT_SOME:
            return LLVMCodeGenLevelDefault;
        case COMPILER_OPT_FAST:
        case COMPILER_OPT_FASTER:
        case COMPILER_OPT_SMALL:
        case COMPILER_OPT_SMALLER:
            return LLVMCodeGenLevelAggressive;
    }
    UNREACHABLE();
}

static LLVMRelocMode getLlvmRelocMode(LlvmCodegenContext* context) {
    if (context->cxt->settings.pie == COMPILER_PIE_NO) {
        return LLVMRelocStatic;
    } else {
        return LLVMRelocPIC;
    }
}

void initLlvmCodegenContext(LlvmCodegenContext* context, CompilerContext* cxt) {
    memset(context, 0, sizeof(LlvmCodegenContext));
    context->cxt = cxt;
    context->llvm_cxt = LLVMContextCreate();
    context->error_msg = NULL;
    const char* target;
    char* llvm_target = NULL;
    if (cxt->settings.target.data == NULL || compareStrings(tocnstr(cxt->settings.target), str("native")) == 0) {
        llvm_target = LLVMGetDefaultTargetTriple();
        target = llvm_target;
    } else {
        target = cstr(cxt->settings.target);
    }
    if (LLVMGetTargetFromTriple(target, &context->target, &context->error_msg)) {
        addMessageToContext(&context->cxt->msgs,createMessage(ERROR_LLVM_BACKEND_ERROR,
            createFormattedString("failed to get target '%s': %s", cstr(context->cxt->settings.target), context->error_msg), 0
        ));
        LLVMDisposeMessage(context->error_msg);
    }
    if (cxt->msgs.error_count == 0) {
        const char* cpu;
        char* llvm_cpu = NULL;
        if (cxt->settings.cpu.data == NULL) {
            cpu = NULL;
        } else if(compareStrings(tocnstr(cxt->settings.cpu), str("native")) == 0) {
            llvm_cpu = LLVMGetHostCPUName();
            cpu = llvm_cpu;
        } else {
            cpu = cstr(cxt->settings.cpu);
        }
        const char* features;
        char* llvm_features = NULL;
        if (cxt->settings.features.data == NULL) {
            features = NULL;
        } else if(compareStrings(tocnstr(cxt->settings.features), str("native")) == 0) {
            llvm_features = LLVMGetHostCPUFeatures();
            features = llvm_features;
        } else {
            features = cstr(cxt->settings.features);
        }
        context->target_machine = LLVMCreateTargetMachine(
            context->target, target, cpu, features, getLlvmCodeGenOptLevel(context),
            getLlvmRelocMode(context), LLVMCodeModelDefault
        );
        LLVMDisposeMessage(llvm_cpu);
        LLVMDisposeMessage(llvm_features);
        context->target_data = LLVMCreateTargetDataLayout(context->target_machine);
    }
    LLVMDisposeMessage(llvm_target);
    context->opaque_type = LLVMStructCreateNamed(context->llvm_cxt, "");
}

void deinitLlvmCodegenContext(LlvmCodegenContext* context) {
    LLVMDisposeTargetData(context->target_data);
    LLVMDisposeTargetMachine(context->target_machine);
    while (context->type_data != NULL) {
        LlvmCodegenData* next = context->type_data->next;
        FREE(context->type_data->struct_mapping);
        FREE(context->type_data);
        context->type_data = next;
    }
    LLVMContextDispose(context->llvm_cxt);
}

LlvmCodegenData* createLlvmCodegenData(LlvmCodegenContext* context) {
    LlvmCodegenData* data = NEW(LlvmCodegenData);
    data->type = NULL;
    data->value = NULL;
    data->debug = NULL;
    data->struct_mapping = NULL;
    data->next = context->type_data;
    context->type_data = data;
    return data;
}

