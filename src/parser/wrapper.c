
#include <errno.h>

#include "parser/parser.tab.h"
#include "parser/lexer.yy.h"
#include "text/format.h"
#include "files/file.h"

#include "parser/wrapper.h"

static AstNode* parseFromFileStream(FILE* stream, const File* file, CompilerContext* msgcontext) {
    ParserContext context = {
        .result = NULL,
        .file = file,
        .context = msgcontext,
        .comment_nesting = 0,
    };
    yyscan_t scanner;
    yylex_init(&scanner);
    yyset_extra(&context, scanner);
    yyset_in(stream, scanner);
    yyparse(scanner, &context);
    yylex_destroy(scanner);
    return context.result;
}

AstNode* parseFile(const File* file, CompilerContext* context) {
    // This is "r+" and not just "r" to get an error when opening directories.
    FILE* in = openFileStream(file, "r+");
    if (in == NULL) {
        addMessageToContext(&context->msgs, createMessage(ERROR_CANT_OPEN_FILE,
            createFormattedString("Failed to open file '%s': %s", cstr(file->original_path), strerror(errno)
        ), 0));
        return NULL;
    } else {
        AstNode* result = parseFromFileStream(in, file, context);
        fclose(in);
        return result;
    }
}

AstNode* parseStdin(CompilerContext* context) {
    return parseFromFileStream(stdin, NULL, context);
}

void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, ParserContext* context, const char* msg) {
    addMessageToContext(&context->context->msgs, createMessage(ERROR_UNKNOWN, copyFromCString(msg), 0));
}

void reportSyntaxError(ParserContext* context, Span loc, const char* actual, size_t num_exp, const char* const* expected) {
    String complete = createFormattedString("Syntax error, unexpected %s", actual);
    if (num_exp > 0) {
        pushFormattedString(&complete, ", expecting %s", expected[0]);
        for (size_t i = 1; i < num_exp; i++) {
            if (i == num_exp - 1) {
                pushFormattedString(&complete, " or %s", expected[i]);
            } else {
                pushFormattedString(&complete, ", %s", expected[i]);
            }
        }
    }
    addMessageToContext(&context->context->msgs, createMessage(ERROR_SYNTAX, complete, 1,
        createMessageFragment(MESSAGE_ERROR, createFormattedString("unexpected %s", actual), loc)
    ));
}

