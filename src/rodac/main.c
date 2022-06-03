
#include <locale.h>
#include <stdlib.h>

#include "ast/astprinter.h"
#include "compiler/varbuild.h"
#include "errors/msgcontext.h"
#include "errors/msgprint.h"
#include "files/fileset.h"
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
            for (size_t i = 0; i < context.files.file_count; i++) {
                File* file = context.files.files[i];
                file->ast = parseFile(file, &context);
                if (context.settings.debug & COMPILER_DEBUG_AST) {
                    printAst(stderr, file->ast);
                }
                if (file->ast != NULL) {
                    buildSymbolTables(&context, file->ast);
                }
                printAndClearMessages(&context.msgs, stderr, true, true);
            }
        }
    }
    printAndClearMessages(&context.msgs, stderr, false, false);
    deinitCompilerContext(&context);
    return EXIT_SUCCESS;
}
