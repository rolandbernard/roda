
#include <stdio.h>

#include "rodac/version.h"
#include "util/params.h"

#include "rodac/params.h"

static PARAM_SPEC_FUNCTION(parameterSpecFunction, CompilerContext* context, {
    PARAM_USAGE(PROGRAM_NAME " [options] files...");
    PARAM_FLAG("h", "help", { context->settings.help = true; }, "print this help information and quit");
    PARAM_FLAG(NULL, "version", { context->settings.version = true; }, "print version information and quit");
    PARAM_FLAG(NULL, "debug", { context->settings.debug = true; }, "print debug information while compiling");
    PARAM_DEFAULT({
        // TODO
    });
    PARAM_WARNING({
        // TODO
    });
})

void printHelpText() {
    PARAM_PRINT_HELP(parameterSpecFunction, NULL);
}

void parseProgramParams(int argc, const char* const* argv, CompilerContext* context) {
    PARAM_PARSE_ARGS(parameterSpecFunction, argc, argv, context);
}

