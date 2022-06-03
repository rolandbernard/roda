
#include <stdio.h>

#include "rodac/version.h"
#include "text/format.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext*, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG("h", "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(NULL, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_VALUED(NULL, "debug", {
        if (value == NULL) {
            PARAM_WARN("expected a value, ignoring this option");
        } else {
            while (value[0] != 0) {
                int len = 0;
                while (value[len] != 0 && value[len] != ',') {
                    len++;
                }
                if (len == 3 && strncmp("all", value, 3) == 0) {
                    context->settings.debug = ~0;
                } else if (len == 3 && strncmp("ast", value, 3) == 0) {
                    context->settings.debug |= COMPILER_DEBUG_AST;
                } else {
                    const char* msg_base = "unexpected value, ignoring this value";
                    char msg[len + strlen(msg_base) + 3];
                    memcpy(msg, value, len);
                    memcpy(msg + len, ": ", 2);
                    memcpy(msg + len + 2, msg_base, strlen(msg_base) + 1);
                    PARAM_WARN(msg);
                }
                if (value[len] == ',') {
                    value += len + 1;
                } else {
                    value += len;
                }
            }
        }
    }, "={all|ast}[,...]", "print debug information while compiling");
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

