
#include <stdlib.h>
#include <locale.h>

#include "parser/wrapper.h"
#include "ast/astprinter.h"

#include "errors/msgcontext.h"
#include "errors/msgprint.h"
#include "text/string.h"
#include "files/fileset.h"

int main(int argc, const char* const* argv) {
    setlocale(LC_ALL, ""); // Set locale to user preference
    FileSet files;
    initFileSet(&files);
    MessageContext context;
    initMessageContext(&context);
    AstNode* ast;
    if (argc > 1) {
        ast = parseFile(createFileInSet(&files, str(argv[1])), &context);
    } else {
        ast = parseStdin(&context);
    }
    printAst(stderr, ast);
    freeAstNode(ast);
    printMessages(&context, stderr, NULL, true, true);
    deinitMessageContext(&context);
    deinitFileSet(&files);
    return EXIT_SUCCESS;
}

