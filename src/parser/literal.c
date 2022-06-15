
#include <string.h>

#include "text/utf8.h"

#include "parser/literal.h"

bool isLiteralParseNoError(LiteralParseError error) {
    return error.offset == NO_POS;
}

static LiteralParseError createLiteralParseError(size_t offset, size_t length) {
    LiteralParseError ret = {
        .offset = offset,
        .length = length,
    };
    return ret;
}

static LiteralParseError createLiteralParseNoError() {
    return createLiteralParseError(NO_POS, NO_POS);
}

static int digitCharToInt(char c) {
    if (c >= '0' && c <= '9') {
        return (int)(c - '0');
    } else if (c >= 'a' && c <= 'z') {
        return (int)(c - 'a') + 10;
    } else if (c >= 'A' && c <= 'Z') {
        return (int)(c - 'A') + 10;
    }
    return -1;
}

static bool isDigitChar(char c, int base) {
    return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
           && digitCharToInt(c) < base;
}

static CodePoint parseEscapeCode(const char* data, size_t* length) {
    CodePoint ret;
    switch (data[0]) {
        case 0:
            *length = 0;
            ret = INVALID_CODEPOINT;
            break;
        case 'a':
            *length = 1;
            ret = '\a';
            break;
        case 'b':
            *length = 1;
            ret = '\b';
            break;
        case 't':
            *length = 1;
            ret = '\t';
            break;
        case 'n':
            *length = 1;
            ret = '\n';
            break;
        case 'v':
            *length = 1;
            ret = '\v';
            break;
        case 'f':
            *length = 1;
            ret = '\f';
            break;
        case 'r':
            *length = 1;
            ret = '\r';
            break;
        case 'e':
            *length = 1;
            ret = '\e';
            break;
        case 'x':
            *length = 3;
            if (isDigitChar(data[1], 16) && isDigitChar(data[2], 16)) {
                ret = (digitCharToInt(data[1]) << 4) | digitCharToInt(data[2]);
            } else {
                ret = INVALID_CODEPOINT;
            }
            break;
        case 'u':
            *length = 5;
            if (isDigitChar(data[1], 16) && isDigitChar(data[2], 16) && isDigitChar(data[3], 16) && isDigitChar(data[4], 16)) {
                ret = (digitCharToInt(data[1]) << 12) | (digitCharToInt(data[2]) << 8)
                      | (digitCharToInt(data[3]) << 4) | digitCharToInt(data[4]);
            } else {
                ret = INVALID_CODEPOINT;
            }
            break;
        case 'U':
            *length = 9;
            if (
                isDigitChar(data[1], 16) && isDigitChar(data[2], 16) && isDigitChar(data[3], 16) && isDigitChar(data[4], 16)
                && isDigitChar(data[5], 16) && isDigitChar(data[6], 16) && isDigitChar(data[7], 16) && isDigitChar(data[8], 16)
            ) {
                ret = (digitCharToInt(data[1]) << 28) | (digitCharToInt(data[2]) << 24)
                      | (digitCharToInt(data[3]) << 20) | (digitCharToInt(data[4]) << 16);
                ret |= (digitCharToInt(data[5]) << 12) | (digitCharToInt(data[6]) << 8)
                      | (digitCharToInt(data[7]) << 4) | digitCharToInt(data[8]);
            } else {
                ret = INVALID_CODEPOINT;
            }
            break;
        default:
            *length = 1;
            ret = data[0];
            break;
    }
    return ret;
}

static LiteralParseError inlineDecodeStringLiteral(String* string) {
    size_t new = 0;
    size_t old = 1;
    string->data[string->length - 1] = 0;
    while (string->data[old] != 0) {
        if (string->data[old] == '\\') {
            size_t length;
            CodePoint codepoint = parseEscapeCode(string->data + old + 1, &length);
            if (codepoint == INVALID_CODEPOINT) {
                if (old + length + 1 < string->length) {
                    return createLiteralParseError(old, length + 1);
                } else {
                    return createLiteralParseError(old, string->length - 1 - old);
                }
            } else {
                new += encodeUTF8(codepoint, string->data + new, length);
                old += length;
            }
            old++;
        } else {
            string->data[new] = string->data[old];
            new++;
            old++;
        }
    }
    string->data[new] = 0;
    return createLiteralParseNoError();
}

LiteralParseError parseStringLiteral(const char* str, String* res) {
    String result = copyFromCString(str);
    if (result.data[0] != '"' || result.data[result.length - 1] != '"') {
        return createLiteralParseError(0, result.length);
    } else {
        LiteralParseError error = inlineDecodeStringLiteral(&result);
        if (isLiteralParseNoError(error)) {
            *res = result;
            return error;
        } else {
            freeString(result);
            return error;
        }
    }
}

LiteralParseError parseCharLiteral(const char* str, CodePoint* res) {
    size_t len = strlen(str);
    if (str[0] != '\'' || str[len - 1] != '\'') {
        return createLiteralParseError(0, len);
    } else {
        CodePoint codepoint;
        size_t len_read;
        size_t col_len;
        if (str[1] == '\\') {
            size_t length;
            codepoint = parseEscapeCode(str + 2, &length);
            len_read = 3 + length;
            col_len = length;
        } else {
            len_read = 2 + decodeUTF8(&codepoint, str + 1, len - 2);
            col_len = getCodePointWidth(codepoint);
        }
        if (len != len_read) {
            return createLiteralParseError(0, len);
        } else {
            if (codepoint == INVALID_CODEPOINT) {
                if (col_len + 1 < len) {
                    return createLiteralParseError(1, col_len + 1);
                } else {
                    return createLiteralParseError(1, len - 1);
                }
            } else {
                *res = codepoint;
            }
            return createLiteralParseNoError();
        }
    }
}

LiteralParseError parseIntLiteral(const char* str, AstIntType* res) {
    AstIntType num = 0;
    size_t idx = 0;
    int base = 10;
    if (str[0] == '0' && str[1] == 'b') {
        base = 2;
        idx += 2;
    } else if (str[0] == '0' && str[1] == 'o') {
        base = 8;
        idx += 2;
    } else if (str[0] == '0' && str[1] == 'd') {
        base = 10;
        idx += 2;
    } else if (str[0] == '0' && (str[1] == 'h' || str[1] == 'x')) {
        base = 16;
        idx += 2;
    }
    while (str[idx] != 0) {
        if (isDigitChar(str[idx], base)) {
            num *= base;
            num += digitCharToInt(str[idx]);
        } else if (str[idx] != '_') {
            return createLiteralParseError(idx, 1);
        }
        idx++;
    }
    *res = num;
    return createLiteralParseNoError();
}

static AstRealType positivePow(AstRealType x, int n) {
    if (n == 0) {
        return 1;
    } else if (n == 1) {
        return x;
    } else if (n % 2 == 1) {
        return x * positivePow(x*x, n / 2);
    } else {
        return positivePow(x*x, n / 2);
    }
}

LiteralParseError parseRealLiteral(const char* str, AstRealType* res) {
    AstRealType num = 0;
    int dec = 0;
    size_t idx = 0;
    while (str[idx] != 0 && str[idx] != '.' && str[idx] != 'e' && str[idx] != 'E') {
        if (isDigitChar(str[idx], 10)) {
            num *= 10;
            num += digitCharToInt(str[idx]);
        } else if (str[idx] != '_') {
            return createLiteralParseError(idx, 1);
        }
        idx++;
    }
    if (str[idx] == '.') {
        idx++;
        while (str[idx] != 0 && str[idx] != 'e' && str[idx] != 'E') {
            if (isDigitChar(str[idx], 10)) {
                dec -= 1;
                num *= 10;
                num += digitCharToInt(str[idx]);
            } else if (str[idx] != '_') {
                return createLiteralParseError(idx, 1);
            }
            idx++;
        }
    }
    if (str[idx] == 'e' || str[idx] == 'E') {
        idx++;
        int neg = false;
        int exp = 0;
        if (str[idx] == '+' || str[idx] == '-') {
            if (str[idx] == '-') {
                neg = true;
            }
            idx++;
        }
        while (str[idx] != 0) {
            if (isDigitChar(str[idx], 10)) {
                exp *= 10;
                exp += digitCharToInt(str[idx]);
            } else if (str[idx] != '_') {
                return createLiteralParseError(idx, 1);
            }
            idx++;
        }
        if (neg) {
            exp = -exp;
        }
        dec += exp;
    }
    if (dec > 0) {
        num *= positivePow(10, dec);
    } else if (dec < 0) {
        num /= positivePow(10, -dec);
    }
    *res = num;
    return createLiteralParseNoError();
}

AstNode* parseStringLiteralIn(ParserContext* ctx, Span loc, const char* str) {
    String result;
    LiteralParseError error = parseStringLiteral(str, &result);
    if (isLiteralParseNoError(error)) {
        return (AstNode*)createAstStr(loc, result);
    } else if (error.length == getSpanLength(loc)) {
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_STR, copyFromCString("string literal is invalid"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid string literal"), loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    } else {
        Span err_loc = loc;
        err_loc.begin = advanceLocationWith(err_loc.begin, str, error.offset);
        err_loc.end = advanceLocationWith(err_loc.begin, str + error.offset, error.length);
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_STR, copyFromCString("string literal contains invalid escape character"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid escape character"), err_loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    }
}

AstNode* parseCharLiteralIn(ParserContext* ctx, Span loc, const char* str) {
    CodePoint result;
    LiteralParseError error = parseCharLiteral(str, &result);
    if (isLiteralParseNoError(error)) {
        return (AstNode*)createAstInt(loc, AST_CHAR, result);
    } else if (error.length == getSpanLength(loc)) {
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_INT, copyFromCString("character literal is invalid"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid character literal"), loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    } else {
        Span err_loc = loc;
        err_loc.begin = advanceLocationWith(err_loc.begin, str, error.offset);
        err_loc.end = advanceLocationWith(err_loc.begin, str + error.offset, error.length);
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_STR, copyFromCString("character literal contains invalid character"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid character"), err_loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    }
}

AstNode* parseIntLiteralIn(ParserContext* ctx, Span loc, const char* str) {
    AstIntType result;
    LiteralParseError error = parseIntLiteral(str, &result);
    if (isLiteralParseNoError(error)) {
        return (AstNode*)createAstInt(loc, AST_INT, result);
    } else {
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_INT, copyFromCString("integer literal is invalid"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid integer literal"), loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    }
}

AstNode* parseRealLiteralIn(ParserContext* ctx, Span loc, const char* str) {
    AstRealType result;
    LiteralParseError error = parseRealLiteral(str, &result);
    if (isLiteralParseNoError(error)) {
        return (AstNode*)createAstReal(loc, result);
    } else {
        addMessageToContext(&ctx->context->msgs, createMessage(ERROR_INVALID_REAL, copyFromCString("real literal is invalid"), 1,
            createMessageFragment(MESSAGE_ERROR, copyFromCString("invalid real literal"), loc)
        ));
        return createAstSimple(loc, AST_ERROR);
    }
}

