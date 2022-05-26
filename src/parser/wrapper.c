
#include "parser/parser.tab.h"

#include "parser/wrapper.h"

AstNode* parseStdin() {
    AstNode* result;
    if (yyparse(&result) != 0) {
        return NULL;
    } else {
        return result;
    }
}

