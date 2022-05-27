
#include "parser/parser.tab.h"
#include "text/format.h"

#include "parser/wrapper.h"

AstNode* parseStdin(MessageContext* context) {
    AstNode* result = NULL;
    yyparse(&result, context);
    return result;
}

void yyerror(AstNode** ast_result, MessageContext* context, const char* s) {
    addMessageToContext(context, createMessage(ERROR_SYNTAX, createFormatedString(s), 0));
}

