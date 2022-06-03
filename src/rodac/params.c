
#include <stdio.h>

#include "rodac/version.h"
#include "text/format.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext*, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG("h", "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(NULL, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_FLAG(NULL, "debug", { context->settings.debug = true; }, "print debug information while compiling");
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

void parseProgramParams(int argc, const char* const* argv, CompilerContext* context) {
    PARAM_PARSE_ARGS(parameterSpecFunction, argc, argv, context);
}

