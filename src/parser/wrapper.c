
#include "parser/parser.tab.h"
#include "parser/lexer.yy.h"
#include "text/format.h"
#include "files/file.h"

#include "parser/wrapper.h"

AstNode* parseFile(File* file, MessageContext* context) {
    AstNode* result = NULL;
    yyin = openFileStream(file, "r");
    yyparse(&result, context);
    fclose(yyin);
    return result;
}

AstNode* parseStdin(MessageContext* context) {
    AstNode* result = NULL;
    yyparse(&result, context);
    return result;
}

void yyerror(AstNode** ast_result, MessageContext* context, const char* s) {
    addMessageToContext(context, createMessage(ERROR_SYNTAX, createFormatedString(s), 0));
}

