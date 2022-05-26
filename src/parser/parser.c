
#include "parser/parser.tab.h"

#include "parser/parser.h"

AstNode* parseStdin() {
    AstNode* result;
    yyparse(&result);
    return result;
}

