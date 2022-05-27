#ifndef _UTF8_H_
#define _UTF8_H_

#include <stdio.h>

#include "text/string.h"

typedef int Rune;

#define INVALID_RUNE -1

int decodeUTF8(Rune* rune, const char* data, int max_length);

int encodeUTF8(Rune rune, char* output, int max_length);

typedef struct {
    ConstString data;
    int offset;
} Utf8Stream;

void initUtf8Stream(Utf8Stream* stream, ConstString string);

void positionUtf8Stream(Utf8Stream* stream, int position);

Rune nextUtf8Rune(Utf8Stream* stream);

Rune peekUtf8Rune(Utf8Stream* stream);

int readUtf8FromFileStream(FILE* file, Rune* out);

int getUtf8Length(ConstString string);

#endif