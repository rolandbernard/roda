#ifndef _FILE_H_
#define _FILE_H_

#include <stdbool.h>
#include <stdio.h>

#include "text/string.h"
#include "files/path.h"

typedef struct {
    ConstString import_path;
    Path original_path;
    Path absolute_path;
    ConstPath directory;
    ConstString name;
    ConstString extention;
} File;

typedef struct {
    File* file;
    int offset;
    int length;
} Span;

void initFile(File* file, ConstString import_path, ConstPath relative_or_absolute_path);

File* copyFile(File* file);

File* createFile(ConstString import_path, ConstPath relative_or_absolute_path);

void deinitFile(File* file);

void freeFile(File* file);

Span createSpan(File* file, int offset, int length);

Span createSpanFromBounds(File* file, int start, int end);

bool isSpanValid(Span span);

bool isSpanFileOnly(Span span);

int getSpanEndOffset(Span span);

bool loadFileData(File* file, String* output);

FILE* openFileStream(File* file, const char* mode);

#endif
