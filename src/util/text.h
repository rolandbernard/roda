#ifndef _UTIL_TEXT_H_
#define _UTIL_TEXT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

bool isHexChar(char c);

uint32_t hexCharToInt(char c);

void inlineDecodeStringLiteral(char* string);

uint32_t parseUTF8(char* c, size_t* length);

size_t printUTF8(uint32_t c, char* out);

#endif
