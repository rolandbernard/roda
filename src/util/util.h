#ifndef _RODA_UTIL_UTIL_H_
#define _RODA_UTIL_UTIL_H_

#include <stdbool.h>

#define MAX(A, B)   ((A) < (B) ? (B) : (A))
#define MIN(A, B)   ((A) > (B) ? (B) : (A))
#define ABS(A)      ((A) < 0 ? -(A) : (A))

bool isDigitChar(char c, int base);

int digitCharToInt(char c);

char digitIntToChar(int i);

#endif
