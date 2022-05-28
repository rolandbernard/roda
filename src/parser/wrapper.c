
#include <errno.h>

#include "parser/parser.tab.h"
#include "parser/lexer.yy.h"
#include "text/format.h"
#include "files/file.h"

#include "parser/wrapper.h"

static AstNode* parseFromFileStream(FILE* file, MessageContext* context) {
    AstNode* result = NULL;
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_in(file, scanner);
    yyparse(scanner, &result, context);
    yylex_destroy(scanner);
    return result;
}

AstNode* parseFile(const File* file, MessageContext* context) {
    FILE* in = openFileStream(file, "r");
    if (in == NULL) {
        addMessageToContext(context, createMessage(ERROR_CANT_OPEN_FILE,
            createFormattedString("Failed to open file '%s': %s", cstr(file->original_path), strerror(errno)
        ), 0));
        return NULL;
    } else {
        AstNode* result = parseFromFileStream(in, context);
        fclose(in);
        return result;
    }
}

AstNode* parseStdin(MessageContext* context) {
    return parseFromFileStream(stdin, context);
}

void yyerror(yyscan_t scanner, AstNode** ast_result, MessageContext* context, const char* s) {
    addMessageToContext(context, createMessage(ERROR_SYNTAX, createFormattedString(s), 0));
}

