
#include <stdint.h>
#include <stdio.h>

#include "rodac/version.h"
#include "text/format.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext*, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG('h', "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(0, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_VALUED('o', "output", {
        if (context->settings.output_file.data != NULL) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->settings.output_file = createPathFromCString(value);
        }
    }, false, "=<file>", "specify the output file");
    PARAM_VALUED(0, "emit", {
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
    PARAM_FLAG('g', "debug", { context->settings.debug = true; }, "include debug information in the output");
    PARAM_VALUED('O', NULL, {
        if (value == NULL) {
            if (context->settings.opt_level != -1 || context->settings.size_level != -1) {
                PARAM_WARN("multiple values for this option, ignoring all but the first");
            } else {
                context->settings.opt_level = 2;
                context->settings.size_level = 1;
            }
        } else if (strlen(value) != 1) {
            PARAM_WARN_UNKNOWN_VALUE()
        } else if (value[0] >= '0' && value[0] <= '3') {
            if (context->settings.opt_level != -1) {
                PARAM_WARN("multiple values for this option, ignoring all but the first");
            } else {
                context->settings.opt_level = value[0] - '0';
            }
        } else if (value[0] == 's' || value[0] == 'z') {
            if (context->settings.size_level != -1) {
                PARAM_WARN("multiple values for this option, ignoring all but the first");
            } else {
                context->settings.opt_level = value[0] == 's' ? 1 : 2;
            }
        } else {
            PARAM_WARN_UNKNOWN_VALUE()
        }
    }, true, "{0|1|2|3|s|z}", "configure the level of optimization to apply");
    PARAM_VALUED(0, "target", {
        if (context->settings.target.data != NULL) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->settings.target = copyFromCString(value);
        }
    }, false, "={<triple>|native}", "specify the compilation target triple");
    PARAM_VALUED(0, "cpu", {
        if (context->settings.target.data != NULL) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->settings.target = copyFromCString(value);
        }
    }, false, "={<cpu>|native}", "specify the compilation target cpu");
    PARAM_VALUED(0, "features", {
        if (context->settings.features.data != NULL) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->settings.features = copyFromCString(value);
        }
    }, false, "={<features>|native}", "specify the compilation target features");
    PARAM_INTEGER(0, "max-errors", {
        if (context->msgfilter.max_errors != SIZE_MAX) {
            PARAM_WARN("multiple values for this option, ignoring all but the first");
        } else {
            context->msgfilter.max_errors = value;
        }
    }, "=<value>", "limit the maximum number of errors to show");
#ifdef DEBUG
    PARAM_STRING_LIST(0, "compiler-debug", {
        if (strcmp("all", value) == 0) {
            context->settings.compiler_debug = ~0;
        } else if (strcmp("log", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_LOG;
        } else if (strcmp("parse-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_PARSE_AST;
        } else if (strcmp("typed-ast", value) == 0) {
            context->settings.compiler_debug |= COMPILER_DEBUG_TYPED_AST;
        } else {
            PARAM_WARN_UNKNOWN_VALUE()
        }
    }, "={all|log|parse-ast|typed-ast}[,...]", "print debug information while compiling");
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

