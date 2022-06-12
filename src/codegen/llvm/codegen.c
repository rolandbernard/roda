
#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/Linker.h>

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

static LLVMModuleRef generateLinkedModule(CompilerContext* context) {
    LLVMModuleRef linked_module = NULL;
    String name = createEmptyString();
    for (size_t i = 0; i < context->files.file_count; i++) {
        pushFormattedString(&name, "%S;", context->files.files[i]->original_path);
    }
    linked_module = LLVMModuleCreateWithName(cstr(name));
    freeString(name);
    for (size_t i = 0; i < context->files.file_count; i++) {
        File* file = context->files.files[i];
        if (file->ast != NULL) {
            LLVMModuleRef module = generateSingleModule(context, file);
            if (module != NULL) {
                if (linked_module != NULL) {
                    if (LLVMLinkModules2(linked_module, module)) {
                        addMessageToContext(
                            &context->msgs,
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

void runCodeGenerationForLlvmIr(CompilerContext* context, ConstPath path) {
    
}

void runCodeGenerationForLlvmBc(CompilerContext* context, ConstPath path) {
    
}

void runCodeGenerationForAsm(CompilerContext* context, ConstPath path) {
    
}

void runCodeGenerationForObj(CompilerContext* context, ConstPath path) {
    
}
