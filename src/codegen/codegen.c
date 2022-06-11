
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "ast/astprinter.h"

#include "codegen/codegen.h"

#define FOR_ALL_MODULES(ACTION)                                 \
    for (size_t i = 0; i < context->files.file_count; i++) {    \
        File* file = context->files.files[i];                   \
        if (file->ast != NULL) ACTION                           \
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
                        break;
                    }
                    case COMPILER_EMIT_LLVM_IR: {
                        break;
                    }
                    case COMPILER_EMIT_LLVM_BC: {
                        break;
                    }
                    case COMPILER_EMIT_ASM: {
                        break;
                    }
                    case COMPILER_EMIT_OBJ: {
                        break;
                    }
                    case COMPILER_EMIT_BIN: {
                        break;
                    }
                }
            }
        }
    }
}

