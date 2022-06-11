
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "ast/astprinter.h"

#ifdef LLVM
#include "codegen/llvm/codegen.h"
#endif

#include "codegen/codegen.h"

#define FOR_ALL_MODULES(ACTION)                                 \
    for (size_t i = 0; i < context->files.file_count; i++) {    \
        File* file = context->files.files[i];                   \
        if (file->ast != NULL) ACTION                           \
    }

void raiseNoBackendError(CompilerContext* context, const char* kind) {
    addMessageToContext(
        &context->msgs,
        createMessage(
            ERROR_NO_OUTPUT_FILE, createFormattedString(
                "cannot compile to %s, program compiled without appropriate backend", kind
            ), 0
        )
    );
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
            FILE* output_file = fopen(cstr(context->settings.output_file), "w");
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
                        remove(cstr(context->settings.output_file));
#ifdef LLVM
                        runCodeGenerationForLlvmIr(context, toConstPath(context->settings.output_file));
#else
                        raiseNoBackendError(context, "LLVM IR");
#endif
                        break;
                    }
                    case COMPILER_EMIT_LLVM_BC: {
                        fclose(output_file);
                        remove(cstr(context->settings.output_file));
#ifdef LLVM
                        runCodeGenerationForLlvmBc(context, toConstPath(context->settings.output_file));
#else
                        raiseNoBackendError(context, "LLVM Bitcode");
#endif
                        break;
                    }
                    case COMPILER_EMIT_ASM: {
                        fclose(output_file);
                        remove(cstr(context->settings.output_file));
#ifdef LLVM
                        runCodeGenerationForAsm(context, toConstPath(context->settings.output_file));
#else
                        raiseNoBackendError(context, "assembly");
#endif
                        break;
                    }
                    case COMPILER_EMIT_OBJ: {
                        fclose(output_file);
                        remove(cstr(context->settings.output_file));
#ifdef LLVM
                        runCodeGenerationForObj(context, toConstPath(context->settings.output_file));
#else
                        raiseNoBackendError(context, "object file");
#endif
                        break;
                    }
                    case COMPILER_EMIT_BIN: {
                        fclose(output_file);
                        // TODO: generate obj + linking
                        break;
                    }
                }
            }
        }
    }
}

