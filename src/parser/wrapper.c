
#include <errno.h>
#include <string.h>

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
            createFormattedString("failed to open file '%s': %s", cstr(file->original_path), strerror(errno)
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

static const char* surroundWith(char* dst, const char* src, char start, char end) {
    const char* ret = dst;
    *dst = start;
    dst++;
    while (*src != 0) {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = end;
    dst++;
    *dst = 0;
    return ret;
}

static const char* tokenName(char* dst, const char* src) {
    if (src[0] == '\'') {
        size_t len = strlen(src);
        dst[0] = '`';
        memcpy(dst + 1, src + 1, len - 2);
        dst[len - 1] = '`';
        dst[len] = 0;
        return dst;
    } else if (
        strcmp(src, "end of file") == 0 || strcmp(src, "invalid token") == 0 || strcmp(src, "identifier") == 0
        || strcmp(src, "string") == 0 || strcmp(src, "integer") == 0 || strcmp(src, "real") == 0
    ) {
        return src;
    } else {
        return surroundWith(dst, src, '`', '`');
    }
}

void reportSyntaxError(ParserContext* context, Span loc, const char* actual, size_t num_exp, const char* const* expected) {
    char name[128]; // This is larger than any defined token name.
    StringBuilder complete;
    initStringBuilder(&complete);
    pushFormattedString(&complete, "syntax error, unexpected %s", tokenName(name, actual));
    if (num_exp > 0) {
        pushFormattedString(&complete, ", expecting %s", tokenName(name, expected[0]));
        for (size_t i = 1; i < num_exp; i++) {
            if (i == num_exp - 1) {
                pushFormattedString(&complete, " or %s", tokenName(name, expected[i]));
            } else {
                pushFormattedString(&complete, ", %s", tokenName(name, expected[i]));
            }
        }
    }
    addMessageToContext(&context->context->msgs, createMessage(ERROR_SYNTAX, builderToString(&complete), 1,
        createMessageFragment(MESSAGE_ERROR, createFormattedString("unexpected %s", tokenName(name, actual)), loc)
    ));
}

