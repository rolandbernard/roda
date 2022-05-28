#ifndef _PARSER_WRAPPER_H_
#define _PARSER_WRAPPER_H_

#include "ast/ast.h"
#include "errors/msgcontext.h"

typedef struct {
    AstNode* result;
    const File* file;
    MessageContext* msgcontext;
} ParserContext;

AstNode* parseFile(const File* file, MessageContext* context);

AstNode* parseStdin(MessageContext* context);

void reportSyntaxError(ParserContext* context, Span loc, const char* actual, size_t num_exp, const char* const* expected);

#endif
