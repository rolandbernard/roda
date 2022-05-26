
#include <stdlib.h>
#include <locale.h>

#include "parser/wrapper.h"
#include "ast/astprinter.h"

int main(int argc, const char* const* argv) {
    setlocale(LC_ALL, ""); // Set locale to user preference
    AstNode* ast = parseStdin();
    printAst(stderr, ast);
    freeAstNode(ast);
    return EXIT_SUCCESS;
}

