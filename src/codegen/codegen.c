
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/astprinter.h"
#include "errors/fatalerror.h"
#include "files/fs.h"
#include "util/debug.h"

#ifdef LLVM
#include "codegen/llvm/codegen.h"
#endif

#include "codegen/codegen.h"

void raiseNoBackendError(CompilerContext* context, const char* kind) {
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_NO_COMPILER_BACKEND, createFormattedString(
                "cannot compile to %s, program compiled without appropriate backend", kind
            ), 0
        )
    );
}

static String getLinkerCommand(CompilerContext* context) {
    String command = copyFromCString("cc");
    if (context->settings.emit_debug) {
        pushFormattedString(&command, " -g");
    }
    if (context->settings.link_type == COMPILER_LINK_STATIC) {
        pushFormattedString(&command, " -static");
    } else if (context->settings.link_type == COMPILER_LINK_SHARED) {
        pushFormattedString(&command, " -shared");
    }
    if (context->settings.pie == COMPILER_PIE_YES) {
        pushFormattedString(&command, " -pie");
    } else if (context->settings.pie == COMPILER_PIE_NO) {
        pushFormattedString(&command, " -no-pie");
    }
    if (context->settings.linker.data != NULL) {
        pushFormattedString(&command, " -fuse-ld=%s", cstr(context->settings.linker));
    }
    if (context->settings.export_dynamic) {
        pushFormattedString(&command, " -rdynamic");
    }
    if (!context->settings.defaultlibs) {
        pushFormattedString(&command, " -nodefaultlibs");
    }
    if (!context->settings.startfiles) {
        pushFormattedString(&command, " -nostartfiles");
    }
    for (size_t i = 0; i < context->settings.lib_dirs.count; i++) {
        pushFormattedString(&command, " -L%s", cstr(context->settings.lib_dirs.strings[i]));
    }
    pushFormattedString(&command, " -o %s", cstr(context->settings.output_file));
    for (size_t i = 0; i < context->settings.objects.count; i++) {
        pushFormattedString(&command, " %s", cstr(context->settings.objects.strings[i]));
    }
    for (size_t i = 0; i < context->settings.libs.count; i++) {
        pushFormattedString(&command, " -l%s", cstr(context->settings.libs.strings[i]));
    }
    return command;
}

void runCodeGeneration(CompilerContext* context) {
    if (context->settings.emit == COMPILER_EMIT_AUTO) {
        if (context->settings.output_file.data == NULL) {
            context->settings.emit = COMPILER_EMIT_NONE;
        } else {
            ConstString ext = getExtention(toConstString(context->settings.output_file));
            if (strcmp("ast", toCString(ext)) == 0) {
                context->settings.emit = COMPILER_EMIT_AST;
            } else if (strcmp("ll", toCString(ext)) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_IR;
            } else if (strcmp("bc", toCString(ext)) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_BC;
            } else if (strcmp("S", toCString(ext)) == 0) {
                context->settings.emit = COMPILER_EMIT_ASM;
            } else if (strcmp("o", toCString(ext)) == 0) {
                context->settings.emit = COMPILER_EMIT_OBJ;
            } else {
                context->settings.emit = COMPILER_EMIT_BIN;
            }
        }
    }
    if (context->settings.emit == COMPILER_EMIT_NONE) {
        if (context->settings.output_file.data != NULL) {
            addMessageToContext(
                &context->msgs,
                createMessage(
                    WARNING_CMD_ARGS,
                    copyFromCString("emit is set to none but an output file is given"), 0
                )
            );
        }
        DEBUG_LOG(context, "skipped code generation");
    } else {
        if (context->settings.output_file.data == NULL) {
            addMessageToContext(
                &context->msgs,
                createMessage(
                    ERROR_NO_OUTPUT_FILE,
                    copyFromCString("no output file specified, but emit specified"), 0
                )
            );
        } else {
            FILE* output_file = openPath(toConstPath(context->settings.output_file), "w");
            if (output_file == NULL) {
                addMessageToContext(
                    &context->msgs,
                    createMessage(
                        ERROR_CANT_OPEN_FILE,
                        createFormattedString(
                            "failed to open file '%s': %s",
                            cstr(context->settings.output_file), strerror(errno)
                        ), 0
                    )
                );
            } else {
                switch (context->settings.emit) {
                    case COMPILER_EMIT_AUTO:
                    case COMPILER_EMIT_NONE: UNREACHABLE()
                    case COMPILER_EMIT_AST: {
                        FOR_ALL_MODULES({ printAst(output_file, file->ast); });
                        fclose(output_file);
                        break;
                    }
                    case COMPILER_EMIT_LLVM_IR: {
                        fclose(output_file);
#ifdef LLVM
                        initLlvmBackend(context);
                        runCodeGenerationForLlvmIr(context, toConstPath(context->settings.output_file));
                        deinitLlvmBackend(context);
#else
                        raiseNoBackendError(context, "LLVM IR");
#endif
                        break;
                    }
                    case COMPILER_EMIT_LLVM_BC: {
                        fclose(output_file);
#ifdef LLVM
                        initLlvmBackend(context);
                        runCodeGenerationForLlvmBc(context, toConstPath(context->settings.output_file));
                        deinitLlvmBackend(context);
#else
                        raiseNoBackendError(context, "LLVM Bitcode");
#endif
                        break;
                    }
                    case COMPILER_EMIT_ASM: {
                        fclose(output_file);
#ifdef LLVM
                        initLlvmBackend(context);
                        runCodeGenerationForAsm(context, toConstPath(context->settings.output_file));
                        deinitLlvmBackend(context);
#else
                        raiseNoBackendError(context, "assembly");
#endif
                        break;
                    }
                    case COMPILER_EMIT_OBJ: {
                        fclose(output_file);
#ifdef LLVM
                        initLlvmBackend(context);
                        runCodeGenerationForObj(context, toConstPath(context->settings.output_file));
                        deinitLlvmBackend(context);
#else
                        raiseNoBackendError(context, "object file");
#endif
                        break;
                    }
                    case COMPILER_EMIT_BIN: {
                        fclose(output_file);
                        Path tmp = getTemporaryFilePath("o");
#ifdef LLVM
                        initLlvmBackend(context);
                        runCodeGenerationForObj(context, toConstPath(tmp));
                        deinitLlvmBackend(context);
#else
                        raiseNoBackendError(context, "object file");
#endif
                        addStringToList(&context->settings.objects, tmp);
                        if (context->msgs.error_count == 0) {
                            String command = getLinkerCommand(context);
                            int ret = system(cstr(command));
                            if (ret < 0) {
                                addMessageToContext(&context->msgs, createMessage(ERROR_LINKER,
                                    copyFromCString("failed to link program, spawning linker failed"), 0
                                ));
                            } else if (ret != 0) {
                                addMessageToContext(&context->msgs, createMessage(ERROR_LINKER, createFormattedString(
                                    "failed to link program, linker exited with non-zero exit code %i", ret
                                ), 0));
                            }
                            freeString(command);
                            DEBUG_LOG(context, "finished running linker");
                        }
                        removePath(toConstPath(tmp));
                        break;
                    }
                }
            }
        }
        DEBUG_LOG(context, "finished code generation");
    }
}

