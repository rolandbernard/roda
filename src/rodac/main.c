
#include <locale.h>
#include <stdlib.h>

#include "compiler/varbuild.h"
#include "ast/astprinter.h"
#include "errors/msgcontext.h"
#include "errors/msgprint.h"
#include "files/fileset.h"
#include "parser/wrapper.h"
#include "text/string.h"

int main(int argc, const char* const* argv) {
    setlocale(LC_ALL, ""); // Set locale to user preference
    CompilerContext context;
    initCompilerContext(&context);
    AstNode* ast;
    if (argc > 1) {
        ast = parseFile(createFileInSet(&context.files, str(argv[1])), &context);
    } else {
        ast = parseStdin(&context);
    }
    printAst(stderr, &context, ast);
    if (ast != NULL) {
        buildSymbolTables(&context, ast);
    }
    freeAstNode(ast);
    printMessages(&context.msgs, stderr, &context.msgfilter, true, true);
    deinitCompilerContext(&context);
    return EXIT_SUCCESS;
}
