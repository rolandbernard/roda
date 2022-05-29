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
    const File* file;
    size_t offset;
    size_t length;
} Span;

void initFile(File* file, ConstPath relative_or_absolute_path);

File* copyFile(const File* file);

File* createFile(ConstPath relative_or_absolute_path);

void deinitFile(File* file);

void freeFile(File* file);

Span invalidSpan();

Span createSpan(const File* file, size_t offset, size_t length);

Span createSpanFromBounds(const File* file, size_t start, size_t end);

Span createSpanWith(Span begin, Span end);

bool isSpanValid(Span span);

bool isSpanFileOnly(Span span);

size_t getSpanEndOffset(Span span);

bool loadFileData(const File* file, String* output);

FILE* openFileStream(const File* file, const char* mode);

#endif
