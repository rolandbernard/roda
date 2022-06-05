
#include <locale.h>
#include <stdlib.h>

#include "compiler/compiler.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"
#include "rodac/params.h"
#include "rodac/version.h"
#include "text/string.h"

static void printVersionInfo() {
    fprintf(stderr, PROGRAM_LONG " v" VERSION_STRING "\n");
}

int main(int argc, const char* const* argv) {
    CompilerContext context;
    initCompilerContext(&context);
    int arg_count = parseProgramParams(argc, argv, &context);
    printAndClearMessages(&context.msgs, stderr, false, false);
    if (context.settings.version || context.settings.help) {
        if (arg_count > 1) {
            addMessageToContext(&context.msgs,
                createMessage(WARNING_CMD_ARGS, copyFromCString("some command line arguments were ignored"), 0)
            );
            printAndClearMessages(&context.msgs, stderr, false, false);
        }
        if (context.settings.version) {
            printVersionInfo();
        } else {
            printHelpText();
        }
    } else {
        if (context.files.file_count == 0) {
            addMessageToContext(&context.msgs,
                createMessage(WARNING_CMD_ARGS, copyFromCString("no input files were given"), 0)
            );
        } else {
            runCompilation(&context);
        }
    }
    printAndClearMessages(&context.msgs, stderr, false, false);
    deinitCompilerContext(&context);
    if (context.msgs.error_count == 0) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
