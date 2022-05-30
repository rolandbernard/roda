#ifndef _UTF8_H_
#define _UTF8_H_

#include <stdint.h>
#include <stdio.h>

#include "text/string.h"

typedef int32_t CodePoint;

#define INVALID_CODEPOINT -1

size_t decodeUTF8(CodePoint* rune, const char* data, size_t max_length);

size_t encodeUTF8(CodePoint rune, char* output, size_t max_length);

typedef struct {
    ConstString data;
    size_t offset;
} Utf8Stream;

void initUtf8Stream(Utf8Stream* stream, ConstString string);

void positionUtf8Stream(Utf8Stream* stream, size_t position);

CodePoint nextUtf8CodePoint(Utf8Stream* stream);

CodePoint peekUtf8CodePoint(const Utf8Stream* stream);

size_t readUtf8FromFileStream(FILE* file, CodePoint* out);

size_t getStringWidth(ConstString string);

size_t getCodePointWidth(CodePoint point);

#endif
