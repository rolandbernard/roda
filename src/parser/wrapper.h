#ifndef _RODA_PARSER_WRAPPER_H_
#define _RODA_PARSER_WRAPPER_H_

#include "ast/ast.h"
#include "compiler/context.h"

typedef struct {
    AstNode* result;
    const File* file;
    CompilerContext* context;
    size_t comment_nesting;
} ParserContext;

AstNode* parseFile(const File* file, CompilerContext* context);

AstNode* parseStdin(CompilerContext* context);

void reportSyntaxError(ParserContext* context, Span loc, const char* actual, size_t num_exp, const char* const* expected);

char* copyToTemporaryString(ParserContext* context, const char* str, size_t length);

void freeTemporaryString(ParserContext* context, char* str);

#endif
