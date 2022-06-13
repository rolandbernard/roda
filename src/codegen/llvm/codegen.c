
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/Linker.h>
#include <llvm-c/TargetMachine.h>

#include "text/format.h"
#include "util/debug.h"
#include "codegen/llvm/genmodule.h"

#include "codegen/llvm/codegen.h"

static void handleLlvmDiagnosticMessage(LLVMDiagnosticInfoRef info, void* udata) {
    CompilerContext* context = (CompilerContext*)udata;
    switch (LLVMGetDiagInfoSeverity(info)) {
        case LLVMDSError: {
            if (applyFilterForKind(&context->msgfilter, ERROR_LLVM_BACKEND_ERROR)) {
                char* desc = LLVMGetDiagInfoDescription(info);
                addMessageToContext(&context->msgs,
                    createMessage(ERROR_LLVM_BACKEND_ERROR, createFormattedString("LLVM backend error: %s", desc), 0)
                );
                LLVMDisposeMessage(desc);
            }
            break;
        }
        case LLVMDSWarning: {
            if (applyFilterForKind(&context->msgfilter, WARNING_LLVM_BACKEND_WARNING)) {
                char* desc = LLVMGetDiagInfoDescription(info);
                addMessageToContext(&context->msgs,
                    createMessage(WARNING_LLVM_BACKEND_WARNING, createFormattedString("LLVM backend warning: %s", desc), 0)
                );
                LLVMDisposeMessage(desc);
            }
            break;
        }
        case LLVMDSRemark: {
            if (applyFilterForKind(&context->msgfilter, NOTE_LLVM_BACKEND_REMARK)) {
                char* desc = LLVMGetDiagInfoDescription(info);
                addMessageToContext(&context->msgs,
                    createMessage(NOTE_LLVM_BACKEND_REMARK, createFormattedString("LLVM backend remark: %s", desc), 0)
                );
                LLVMDisposeMessage(desc);
            }
            break;
        }
        case LLVMDSNote: {
            if (applyFilterForKind(&context->msgfilter, NOTE_LLVM_BACKEND_NOTE)) {
                char* desc = LLVMGetDiagInfoDescription(info);
                addMessageToContext(&context->msgs,
                    createMessage(NOTE_LLVM_BACKEND_NOTE, createFormattedString("LLVM backend note: %s", desc), 0)
                );
                LLVMDisposeMessage(desc);
            }
            break;
        }
    }
}

void initLlvmBackend(CompilerContext* context) {
    LLVMInitializeCore(LLVMGetGlobalPassRegistry());
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllTargetMCs();
    LLVMContextRef llvm_context = LLVMGetGlobalContext();
    LLVMContextSetDiagnosticHandler(llvm_context, handleLlvmDiagnosticMessage, context);
    DEBUG_LOG(context, "initialized LLVM backend");
}

void deinitLlvmBackend(CompilerContext* context) {
    LLVMShutdown();
    DEBUG_LOG(context, "shutdown LLVM backend");
}

static LLVMCodeGenOptLevel getLlvmCodeGenOptLevel(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case 0:
            return LLVMCodeGenLevelNone;
        case 1:
            return LLVMCodeGenLevelDefault;
        case 2:
            return LLVMCodeGenLevelDefault;
        case 3:
            return LLVMCodeGenLevelAggressive;
        default:
            return context->cxt->settings.size_level == 2
                ? LLVMCodeGenLevelAggressive
                : LLVMCodeGenLevelDefault;
    }
}

static LLVMRelocMode getLlvmRelocMode(LlvmCodegenContext* context) {
    // TODO: make this a command line option?
    return LLVMRelocPIC;
}

static void initLlvmCodegenContext(LlvmCodegenContext* context, CompilerContext* cxt) {
    memset(context, 0, sizeof(LlvmCodegenContext));
    context->cxt = cxt;
    context->llvm_cxt = LLVMGetGlobalContext();
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
            createFormattedString("failed to get target '%S': %s", context->cxt->settings.target, context->error_msg), 0
        ));
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
}

static void deinitLlvmCodegenContext(LlvmCodegenContext* context) {
    LLVMDisposeTargetData(context->target_data);
    LLVMDisposeTargetMachine(context->target_machine);
}

static LLVMModuleRef generateLinkedModule(LlvmCodegenContext* context) {
    LLVMModuleRef linked_module = NULL;
    String name = createEmptyString();
    for (size_t i = 0; i < context->cxt->files.file_count; i++) {
        pushFormattedString(&name, "%S;", context->cxt->files.files[i]->original_path);
    }
    linked_module = LLVMModuleCreateWithName(cstr(name));
    freeString(name);
    for (size_t i = 0; i < context->cxt->files.file_count; i++) {
        File* file = context->cxt->files.files[i];
        if (file->ast != NULL) {
            LLVMModuleRef module = generateSingleModule(context, file);
            if (module != NULL) {
                if (linked_module != NULL) {
                    if (LLVMLinkModules2(linked_module, module)) {
                        addMessageToContext(
                            &context->cxt->msgs,
                            createMessage(
                                ERROR_LLVM_BACKEND_ERROR, createFormattedString(
                                    "failed to link in module '%S'", file->original_path
                                ), 0
                            )
                        );
                    }
                } else {
                    linked_module = module;
                }
            }
        }
    }
    return linked_module;
}

static LLVMModuleRef generateOptimizedModule(LlvmCodegenContext* context) {
    LLVMModuleRef module = generateLinkedModule(context);
    // TODO: implement optimization
    return module;
}

void runCodeGenerationForLlvmIr(CompilerContext* cxt, ConstPath path) {
    LlvmCodegenContext context;
    initLlvmCodegenContext(&context, cxt);
    if (cxt->msgs.error_count == 0) {
        LLVMModuleRef module = generateOptimizedModule(&context);
        if (LLVMPrintModuleToFile(module, toCString(path), &context.error_msg)) {
            addMessageToContext(
                &cxt->msgs,
                createMessage(
                    ERROR_LLVM_BACKEND_ERROR,
                    createFormattedString("failed to write output file '%S': %s", path, context.error_msg), 0
                )
            );
            LLVMDisposeMessage(context.error_msg);
        }
        LLVMDisposeModule(module);
    }
    deinitLlvmCodegenContext(&context);
}

void runCodeGenerationForLlvmBc(CompilerContext* context, ConstPath path) {
    
}

void runCodeGenerationForAsm(CompilerContext* context, ConstPath path) {
    
}

void runCodeGenerationForObj(CompilerContext* context, ConstPath path) {
    
}
