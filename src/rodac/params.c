
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
            context->settings.debug = ~0;
        } else if (strcmp("parse-ast", value) == 0) {
            context->settings.debug |= COMPILER_DEBUG_PARSE_AST;
        } else if (strcmp("typed-ast", value) == 0) {
            context->settings.debug |= COMPILER_DEBUG_TYPED_AST;
        } else {
            const char* msg_base = "unexpected value, ignoring this value";
            char msg[len + strlen(msg_base) + 3];
            memcpy(msg, value, len);
            memcpy(msg + len, ": ", 2);
            memcpy(msg + len + 2, msg_base, strlen(msg_base) + 1);
            PARAM_WARN(msg);
        }
    }, "={all|parse-ast|typed-ast}[,...]", "print debug information while compiling");
#endif
    PARAM_DEFAULT({
        createFileInSet(&context->files, str(value));
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

