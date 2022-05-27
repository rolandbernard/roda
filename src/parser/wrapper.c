
#include "parser/parser.tab.h"

#include "parser/wrapper.h"

AstNode* parseStdin() {
    AstNode* result = NULL;
    yyparse(&result);
    return result;
}

