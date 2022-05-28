#ifndef _FORMAT_H_
#define _FORMAT_H_

#include "text/string.h"

String createFormattedString(const char* format, ...);

void pushFormattedString(String* dst, const char* format, ...);

#endif
