
#include <stdlib.h>
#include <locale.h>

#include "parser/parser.tab.h"

int main(int argc, const char* const* argv) {
    setlocale(LC_ALL, ""); // Set locale to user preference
    yyparse();
    return EXIT_SUCCESS;
}

