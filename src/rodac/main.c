
#include <locale.h>
#include <stdlib.h>

#include "compiler/compiler.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"
#include "rodac/params.h"
#include "rodac/version.h"
#include "text/string.h"
#include "util/debug.h"

static void printVersionInfo() {
    fprintf(stderr, PROGRAM_LONG " v" VERSION_STRING "\n");
    fprintf(stderr, "  version: " VERSION_STRING_BUILD "\n");
#ifdef GIT_URL
    fprintf(stderr, "  git: " GIT_URL "\n");
#endif
#ifdef LLVM
    fprintf(stderr, "  LLVM backend: yes\n");
#else
    fprintf(stderr, "  LLVM backend: no\n");
#endif
}

int main(int argc, const char* const* argv) {
    CompilerContext context;
    initCompilerContext(&context);
    int arg_count = parseProgramParams(argc, argv, &context);
    printAndClearMessages(&context.msgs, stderr, false, false);
    DEBUG_LOG(&context, "finished parsing command line arguments");
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
        DEBUG_LOG(&context, "compilation finished without errors");
        return EXIT_SUCCESS;
    } else {
        DEBUG_LOG(&context, "compilation finished with errors");
        return EXIT_FAILURE;
    }
}
