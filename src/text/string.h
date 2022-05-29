#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

#define mutstr(CSTR) createFromCString(CSTR)
#define str(CSTR) createFromConstCString(CSTR)
#define cstr(STR) toCString(toConstString(STR))
#define tocnstr(STR) toConstString(STR)

#define NO_POS (size_t)-1

typedef struct {
    char* data;
    size_t length;
} String;

typedef struct {
    const char* data;
    size_t length;
} ConstString;

size_t findFirstIndexOfChar(ConstString string, char c);

size_t findFirstIndexOfString(ConstString string, ConstString pattern);

size_t findLastIndexOfChar(ConstString string, char c);

size_t findLastIndexOfString(ConstString string, ConstString pattern);

ConstString getStringBeforeChar(ConstString string, char c);

ConstString getStringAfterChar(ConstString string, char c);

String createString(char* data, size_t length);

ConstString createConstString(const char* data, size_t length);

String createFromCString(char* data);

ConstString createFromConstCString(const char* data);

String createEmptyString();

ConstString createEmptyConstString();

String copyString(ConstString string);

String concatStrings(ConstString a, ConstString b);

String concatNStrings(size_t n, ...);

String toNonConstString(ConstString string);

ConstString toConstString(String string);

int compareStrings(ConstString a, ConstString b);

void freeString(String string);

const char* toCString(ConstString string);

String resizeStringData(String string);

String copyFromCString(const char* cstr);

String pushToString(String dst, ConstString src);

#endif
