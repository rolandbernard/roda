
#include <stdint.h>
#include <stdio.h>

#include "rodac/version.h"
#include "text/format.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext*, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG("h", "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(NULL, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_VALUED("o", "output", {
        if (context->settings.output_file.data != NULL) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->settings.output_file = createPathFromCString(value);
        }
    }, false, "=<file>", "specify the output file");
    PARAM_VALUED(NULL, "emit", {
        if (context->settings.emit != COMPILER_EMIT_AUTO) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            if (strcmp("none", value) == 0) {
                context->settings.emit = COMPILER_EMIT_NONE;
            } else if (strcmp("ast", value) == 0) {
                context->settings.emit = COMPILER_EMIT_AST;
            } else if (strcmp("llvm-ir", value) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_IR;
            } else if (strcmp("llvm-bc", value) == 0) {
                context->settings.emit = COMPILER_EMIT_LLVM_BC;
            } else if (strcmp("asm", value) == 0) {
                context->settings.emit = COMPILER_EMIT_ASM;
            } else if (strcmp("obj", value) == 0) {
                context->settings.emit = COMPILER_EMIT_OBJ;
            } else if (strcmp("bin", value) == 0) {
                context->settings.emit = COMPILER_EMIT_BIN;
            } else {
                PARAM_WARN_UNKNOWN_VALUE()
            }
        }
    }, false, "={none|ast|llvm-ir|llvm-bc|asm|obj|bin}", "select what the compiler should emit");
    PARAM_INTEGER(NULL, "max-errors", {
        if (context->msgfilter.max_errors != SIZE_MAX) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->msgfilter.max_errors = value;
        }
    }, "=<value>", "limit the maximum number of errors to show");
#ifdef DEBUG
    PARAM_STRING_LIST(NULL, "compiler-debug", {
        if (strcmp("all", value) == 0) {
            context->settings.compiler_debug = ~0;
        } else if (strcmp("parse-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_PARSE_AST;
        } else if (strcmp("typed-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_TYPED_AST;
        } else {
            PARAM_WARN_UNKNOWN_VALUE()
        }
    }, "={all|parse-ast|typed-ast}[,...]", "print debug information while compiling");
#endif
    PARAM_DEFAULT({
        Path path = createPathFromCString(value);
        createFileInSet(&context->files, toConstPath(path));
        freePath(path);
    });
    PARAM_WARNING({
        addMessageToContext(&context->msgs, createMessage(
            WARNING_CMD_ARGS, createFormattedString("%s: %s", option, warning), 0
        ));
    });
})

void printHelpText() {
    PARAM_PRINT_HELP(parameterSpecFunction, NULL);
}

int parseProgramParams(int argc, const char* const* argv, CompilerContext* context) {
    return PARAM_PARSE_ARGS(parameterSpecFunction, argc, argv, context);
}

