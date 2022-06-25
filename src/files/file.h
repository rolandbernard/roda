#ifndef _RODA_FILES_FILE_H_
#define _RODA_FILES_FILE_H_

#include <stdbool.h>
#include <stdio.h>

#include "text/string.h"
#include "files/path.h"

typedef enum {
    FILE_RODA,
    FILE_LLVM_IR,
    FILE_LLVM_BC,
    FILE_OBJECT,
} FileType;

typedef struct File {
    struct File* next;
    FileType type;
    Path original_path;
    Path absolute_path;
    ConstPath directory;
    ConstString name;
    ConstString extention;
    struct AstNode* ast;
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

Location invalidLocation();

Span invalidSpan();

bool isSpanValid(Span span);

bool isSpanFileOnly(Span span);

bool isSpanWithPosition(Span span);

Span createFileOnlySpan(const File* file);

Span combineSpans(Span begin, Span end);

size_t getSpanLength(Span span);

size_t getSpanColumnLength(Span span);

Location advanceLocationWith(Location start, const char* text, size_t len);

bool loadFileData(const File* file, String* output);

FILE* openFileStream(const File* file, const char* mode);

#endif
