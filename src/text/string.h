#ifndef _STRING_H_
#define _STRING_H_

#define mutstr(CSTR) createFromCString(CSTR)
#define str(CSTR) createFromConstCString(CSTR)
#define cstr(STR) toCString(toConstString(STR))

typedef struct {
    char* data;
    int length;
} String;

typedef struct {
    const char* data;
    int length;
} ConstString;

int findFirstIndexOfChar(ConstString string, char c);

int findFirstIndexOfString(ConstString string, ConstString pattern);

int findLastIndexOfChar(ConstString string, char c);

int findLastIndexOfString(ConstString string, ConstString pattern);

ConstString getStringBeforeChar(ConstString string, char c);

ConstString getStringAfterChar(ConstString string, char c);

String createString(char* data, int length);

ConstString createConstString(const char* data, int length);

String createFromCString(char* data);

ConstString createFromConstCString(const char* data);

String createEmptyString();

ConstString createEmptyConstString();

String copyString(ConstString string);

String concatStrings(ConstString a, ConstString b);

String concatNStrings(int n, ...);

String cloneCString(const char* cstr);

String toNonConstString(ConstString string);

ConstString toConstString(String string);

int compareStrings(ConstString a, ConstString b);

void freeString(String string);

const char* toCString(ConstString string);

String resizeStringData(String string);

String copyFromCString(const char* cstr);

void inlineDecodeStringLiteral(String* string);

#endif
