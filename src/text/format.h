#ifndef _RODA_TEXT_FORMAT_H_
#define _RODA_TEXT_FORMAT_H_

#include "text/string.h"

String createFormattedString(const char* format, ...);

void pushFormattedString(StringBuilder* dst, const char* format, ...);

#endif
