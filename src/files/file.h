#ifndef _FILE_H_
#define _FILE_H_

#include <stdbool.h>
#include <stdio.h>

#include "text/string.h"
#include "files/path.h"

typedef struct {
    Path original_path;
    Path absolute_path;
    ConstPath directory;
    ConstString name;
    ConstString extention;
} File;

typedef struct {
    size_t offset;
    size_t line;
    size_t column;
} Location;

typedef struct {
    const File* file;
    Location begin;
    Location end;
} Span;

void initFile(File* file, ConstPath relative_or_absolute_path);

File* copyFile(const File* file);

File* createFile(ConstPath relative_or_absolute_path);

void deinitFile(File* file);

void freeFile(File* file);

Span invalidSpan();

bool isSpanValid(Span span);

bool isSpanFileOnly(Span span);

bool isSpanWithPosition(Span span);

Span createStartSpan(const File* file);

Span combineSpans(Span begin, Span end);

size_t getSpanLength(Span span);

Location advanceLocationWith(Location start, const char* text, size_t len);

bool loadFileData(const File* file, String* output);

FILE* openFileStream(const File* file, const char* mode);

#endif
