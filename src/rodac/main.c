
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
    setlocale(LC_ALL, ""); // Set locale to user preference
    CompilerContext context;
    initCompilerContext(&context);
    parseProgramParams(argc, argv, &context);
    if (context.settings.version) {
        printVersionInfo();
    } else if (context.settings.help) {
        printHelpText();
    } else {
        AstNode* ast;
        if (argc > 1) {
            ast = parseFile(createFileInSet(&context.files, str(argv[1])), &context);
        } else {
            ast = parseStdin(&context);
        }
        printAst(stderr, ast);
        if (ast != NULL) {
            buildSymbolTables(&context, ast);
        }
        freeAstNode(ast);
        printMessages(&context.msgs, stderr, true, true);
    }
    deinitCompilerContext(&context);
    return EXIT_SUCCESS;
}
