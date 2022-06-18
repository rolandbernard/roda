
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/Linker.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <string.h>

#if LLVM_VERSION_MAJOR >= 13
#include <llvm-c/Transforms/PassBuilder.h>
#else
#include <llvm-c/Transforms/PassManagerBuilder.h>
#endif

#include "codegen/llvm/genmodule.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "util/alloc.h"
#include "util/debug.h"

#include "codegen/llvm/codegen.h"

static void trimErrorMessage(char* data) {
    size_t len = strlen(data);
    while (data[len - 1] == '\n') {
        len--;
    }
    data[len] = 0;
}

static void handleLlvmDiagnosticMessage(LLVMDiagnosticInfoRef info, void* udata) {
    CompilerContext* context = (CompilerContext*)udata;
    switch (LLVMGetDiagInfoSeverity(info)) {
        case LLVMDSError: {
            if (applyFilterForKind(&context->msgfilter, ERROR_LLVM_BACKEND_ERROR)) {
                char* desc = LLVMGetDiagInfoDescription(info);
                trimErrorMessage(desc);
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
                trimErrorMessage(desc);
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
                trimErrorMessage(desc);
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
                trimErrorMessage(desc);
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

static LLVMModuleRef generateLinkedModule(LlvmCodegenContext* context) {
    LLVMModuleRef linked_module = NULL;
    String name = createEmptyString();
    for (size_t i = 0; i < context->cxt->files.file_count; i++) {
        pushFormattedString(&name, "%S;", context->cxt->files.files[i]->original_path);
    }
    linked_module = LLVMModuleCreateWithName(cstr(name));
    freeString(name);
    LLVMSetModuleDataLayout(linked_module, context->target_data);
    for (size_t i = 0; i < context->cxt->files.file_count; i++) {
        File* file = context->cxt->files.files[i];
        if (file->ast != NULL) {
            LLVMModuleRef module = generateSingleModule(context, file);
#ifdef DEBUG
    if (LLVMVerifyModule(module, LLVMReturnStatusAction, &context->error_msg)) {
        trimErrorMessage(context->error_msg);
        addMessageToContext(
            &context->cxt->msgs,
            createMessage(
                ERROR_LLVM_BACKEND_ERROR,
                createFormattedString("generated an invalid llvm module: %s", context->error_msg), 0
            )
        );
    }
    LLVMDisposeMessage(context->error_msg);
#endif
            if (module != NULL) {
                /* if (linked_module != NULL) { */
                /*     if (LLVMLinkModules2(linked_module, module)) { */
                /*         addMessageToContext( */
                /*             &context->cxt->msgs, */
                /*             createMessage( */
                /*                 ERROR_LLVM_BACKEND_ERROR, createFormattedString( */
                /*                     "failed to link in module '%S'", file->original_path */
                /*                 ), 0 */
                /*             ) */
                /*         ); */
                /*     } */
                /* } else { */
                    linked_module = module;
                /* } */
            }
        }
    }
    return linked_module;
}

#if LLVM_VERSION_MAJOR >= 13
static const char* getLlvmPassPipeline(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case COMPILER_OPT_DEFAULT:
            return context->cxt->settings.emit_debug ? "default<O0>" : "default<O2>";
        case COMPILER_OPT_NONE:
            return "default<O0>";
        case COMPILER_OPT_SOME:
            return "default<O1>";
        case COMPILER_OPT_FAST:
            return "default<O2>";
        case COMPILER_OPT_FASTER:
            return "default<O3>";
        case COMPILER_OPT_SMALL:
            return "default<Os>";
        case COMPILER_OPT_SMALLER:
            return "default<Oz>";
    }
    UNREACHABLE();
}

static void optimizeUsingNewPassManager(LlvmCodegenContext* context, LLVMModuleRef module) {
    LLVMPassBuilderOptionsRef options = LLVMCreatePassBuilderOptions();
    LLVMErrorRef error = LLVMRunPasses(module, getLlvmPassPipeline(context), context->target_machine, options);
    if (error != NULL) {
        context->error_msg = LLVMGetErrorMessage(error);
        addMessageToContext(
            &context->cxt->msgs,
            createMessage(
                ERROR_LLVM_BACKEND_ERROR,
                createFormattedString("failed an optimize llvm module: %s", context->error_msg), 0
            )
        );
        LLVMDisposeErrorMessage(context->error_msg);
    }
    LLVMDisposePassBuilderOptions(options);
}
#else
static int getLlvmPassInlineThreshold(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case COMPILER_OPT_DEFAULT:
            return context->cxt->settings.emit_debug ? 0 : 200;
        case COMPILER_OPT_NONE:
            return 0;
        case COMPILER_OPT_SOME:
            return 100;
        case COMPILER_OPT_FAST:
            return 200;
        case COMPILER_OPT_FASTER:
            return 1000;
        case COMPILER_OPT_SMALL:
            return 200;
        case COMPILER_OPT_SMALLER:
            return 100;
    }
    UNREACHABLE();
}

static int getLlvmOptLevel(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case COMPILER_OPT_DEFAULT:
            return context->cxt->settings.emit_debug ? 0 : 2;
        case COMPILER_OPT_NONE:
            return 0;
        case COMPILER_OPT_SOME:
            return 1;
        case COMPILER_OPT_FAST:
            return 2;
        case COMPILER_OPT_FASTER:
            return 3;
        case COMPILER_OPT_SMALL:
        case COMPILER_OPT_SMALLER:
            return 2;
    }
    UNREACHABLE();
}

static int getLlvmSizeLevel(LlvmCodegenContext* context) {
    switch (context->cxt->settings.opt_level) {
        case COMPILER_OPT_DEFAULT:
        case COMPILER_OPT_NONE:
        case COMPILER_OPT_SOME:
        case COMPILER_OPT_FAST:
        case COMPILER_OPT_FASTER:
            return 0;
        case COMPILER_OPT_SMALL:
            return 1;
        case COMPILER_OPT_SMALLER:
            return 2;
    }
    UNREACHABLE();
}

static void optimizeUsingLegacyPassManager(LlvmCodegenContext* context, LLVMModuleRef module) {
    LLVMPassManagerRef module_pass_manager = LLVMCreatePassManager();
    LLVMPassManagerBuilderRef pass_manager_builder = LLVMPassManagerBuilderCreate();
    LLVMPassManagerBuilderSetOptLevel(pass_manager_builder, getLlvmOptLevel(context));
    LLVMPassManagerBuilderSetSizeLevel(pass_manager_builder, getLlvmSizeLevel(context));
    LLVMPassManagerBuilderUseInlinerWithThreshold(pass_manager_builder, getLlvmPassInlineThreshold(context));
    LLVMAddAnalysisPasses(context->target_machine, module_pass_manager);
    LLVMPassManagerBuilderPopulateModulePassManager(pass_manager_builder, module_pass_manager);
    LLVMRunPassManager(module_pass_manager, module);
    LLVMDisposePassManager(module_pass_manager);
    LLVMPassManagerBuilderDispose(pass_manager_builder);
}
#endif

static LLVMModuleRef generateOptimizedModule(LlvmCodegenContext* context) {
    LLVMModuleRef module = generateLinkedModule(context);
    if (context->cxt->settings.opt_level != COMPILER_OPT_NONE) {
#if LLVM_VERSION_MAJOR >= 13
        optimizeUsingNewPassManager(context, module);
#else
        optimizeUsingLegacyPassManager(context, module);
#endif
    }
    return module;
}

void runCodeGenerationForLlvmIr(CompilerContext* cxt, ConstPath path) {
    LlvmCodegenContext context;
    initLlvmCodegenContext(&context, cxt);
    if (cxt->msgs.error_count == 0) {
        LLVMModuleRef module = generateOptimizedModule(&context);
        if (LLVMPrintModuleToFile(module, toCString(path), &context.error_msg)) {
            trimErrorMessage(context.error_msg);
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

void runCodeGenerationForLlvmBc(CompilerContext* cxt, ConstPath path) {
    LlvmCodegenContext context;
    initLlvmCodegenContext(&context, cxt);
    if (cxt->msgs.error_count == 0) {
        LLVMModuleRef module = generateOptimizedModule(&context);
        if (LLVMWriteBitcodeToFile(module, toCString(path))) {
            addMessageToContext(
                &cxt->msgs,
                createMessage(
                    ERROR_LLVM_BACKEND_ERROR, 
                    createFormattedString("failed to write output file '%S'", path), 0
                )
            );
        }
        LLVMDisposeModule(module);
    }
    deinitLlvmCodegenContext(&context);
}

static void runCodeGenerationForTargetMachine(CompilerContext* cxt, ConstPath path, LLVMCodeGenFileType type) {
    LlvmCodegenContext context;
    initLlvmCodegenContext(&context, cxt);
    if (cxt->msgs.error_count == 0) {
        LLVMModuleRef module = generateOptimizedModule(&context);
        char* filename = copyToCString(path);
        if (LLVMTargetMachineEmitToFile(context.target_machine, module, filename, type, &context.error_msg)) {
            trimErrorMessage(context.error_msg);
            addMessageToContext(
                &cxt->msgs,
                createMessage(
                    ERROR_LLVM_BACKEND_ERROR,
                    createFormattedString("failed to write output file '%S': %s", path, context.error_msg), 0
                )
            );
            LLVMDisposeMessage(context.error_msg);
        }
        FREE(filename);
        LLVMDisposeModule(module);
    }
    deinitLlvmCodegenContext(&context);
}

void runCodeGenerationForAsm(CompilerContext* cxt, ConstPath path) {
    runCodeGenerationForTargetMachine(cxt, path, LLVMAssemblyFile);
}

void runCodeGenerationForObj(CompilerContext* cxt, ConstPath path) {
    runCodeGenerationForTargetMachine(cxt, path, LLVMObjectFile);
}

