#ifndef _TEXT_PARSE_H_
#define _TEXT_PARSE_H_

#include <stdbool.h>

#include "ast/ast.h"
#include "parser/wrapper.h"

typedef struct {
    size_t offset;
    size_t length;
} LiteralParseError;

bool isLiteralParseNoError(LiteralParseError error);

LiteralParseError parseStringLiteral(const char* str, String* res);

LiteralParseError parseIntLiteral(const char* str, AstIntType* res);

LiteralParseError parseRealLiteral(const char* str, AstRealType* res);

AstNode* parseStringLiteralIn(ParserContext* ctx, Span loc, const char* str);

AstNode* parseIntLiteralIn(ParserContext* ctx, Span loc, const char* str);

AstNode* parseRealLiteralIn(ParserContext* ctx, Span loc, const char* str);

#endif
