
#include <errno.h>

#include "parser/parser.tab.h"
#include "parser/lexer.yy.h"
#include "text/format.h"
#include "files/file.h"

#include "parser/wrapper.h"

static AstNode* parseFromFileStream(FILE* stream, const File* file, MessageContext* msgcontext) {
    ParserContext context = {
        .result = NULL,
        .file = file,
        .msgcontext = msgcontext,
    };
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_extra(&context, scanner);
    yyset_in(stream, scanner);
    yyparse(scanner, &context);
    yylex_destroy(scanner);
    return context.result;
}

AstNode* parseFile(const File* file, MessageContext* context) {
    // This is "r+" and not just "r" to get an error when opening directories.
    FILE* in = openFileStream(file, "r+");
    if (in == NULL) {
        addMessageToContext(context, createMessage(ERROR_CANT_OPEN_FILE,
            createFormattedString("Failed to open file '%s': %s", cstr(file->original_path), strerror(errno)
        ), 0));
        return NULL;
    } else {
        AstNode* result = parseFromFileStream(in, file, context);
        fclose(in);
        return result;
    }
}

AstNode* parseStdin(MessageContext* context) {
    return parseFromFileStream(stdin, NULL, context);
}

void yyerror(yyscan_t scanner, ParserContext* context, const char* s) {
    addMessageToContext(context->msgcontext, createMessage(ERROR_SYNTAX, createFormattedString(s), 0));
}

