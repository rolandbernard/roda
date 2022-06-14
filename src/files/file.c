
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast/ast.h"
#include "errors/fatalerror.h"
#include "files/fs.h"
#include "text/string.h"
#include "text/utf8.h"
#include "util/alloc.h"

#include "files/file.h"

void initFile(File* file, ConstPath file_path) {
    file->original_path = copyPath(file_path);
    file->absolute_path = getAbsolutePath(file_path);
    file->directory = getParentDirectory(toConstPath(file->absolute_path));
    file->extention = getExtention(toConstPath(file->absolute_path));
    file->name = getFilename(toConstPath(file->absolute_path));
    file->ast = NULL;
}

File* copyFile(const File* file) {
    File* ret = ALLOC(File, 1);
    ret->original_path = copyPath(toConstPath(file->original_path));
    ret->absolute_path = copyPath(toConstPath(file->absolute_path));
    ret->directory = getParentDirectory(toConstPath(ret->absolute_path));
    ret->extention = getExtention(toConstPath(ret->absolute_path));
    ret->name = getFilename(toConstPath(ret->absolute_path));
    return ret;
}

File* createFile(ConstPath file_path) {
    File* ret = NEW(File);
    initFile(ret, file_path);
    return ret;
}

void deinitFile(File* file) {
    freePath(file->original_path);
    freePath(file->absolute_path);
    freeAstNode(file->ast);
}

void freeFile(File* file) {
    deinitFile(file);
    FREE(file);
}

Location invalidLocation() {
    Location ret = { .offset = NO_POS, .line = NO_POS, .column = NO_POS };
    return ret;
}

Span invalidSpan() {
    Span ret = { .file = NULL, .begin = invalidLocation(), .end = invalidLocation() };
    return ret;
}

bool isSpanValid(Span span) {
    return span.file != NULL || span.begin.offset != NO_POS;
}

bool isSpanFileOnly(Span span) {
    return isSpanValid(span) && span.file != NULL;
}

bool isSpanWithPosition(Span span) {
    return isSpanValid(span) && span.begin.offset != NO_POS;
}

Span createFileOnlySpan(const File* file) {
    Span ret = invalidSpan();
    ret.file = file;
    return ret;
}

Span combineSpans(Span begin, Span end) {
    Span ret = {
        .file = begin.file != NULL ? begin.file : end.file,
        .begin = begin.begin,
        .end = end.end,
    };
    return ret;
}

size_t getSpanLength(Span span) {
    return span.end.offset - span.begin.offset;
}

size_t getSpanColumnLength(Span span) {
    if (span.end.column < span.begin.column) {
        return 0;
    } else {
        return span.end.column - span.begin.column;
    }
}

Location advanceLocationWith(Location start, const char* text, size_t len) {
    Utf8Stream stream;
    initUtf8Stream(&stream, createConstString(text, len));
    while (stream.offset < len) {
        CodePoint p = nextUtf8CodePoint(&stream);
        if (p == '\n') {
            start.line += 1;
            start.column = 0;
        } else {
            start.column += getCodePointWidth(p);
        }
    }
    start.offset += stream.data.length;
    return start;
}

bool loadFileData(const File* file, String* output) {
    FILE* stream = openPath(toConstPath(file->absolute_path), "r");
    if (stream == NULL) {
        return false;
    } else {
        fseek(stream, 0, SEEK_END);
        output->length = ftell(stream);
        fseek(stream, 0, SEEK_SET);
        output->length -= ftell(stream);
        output->data = ALLOC(char, output->length);
        fread(output->data, 1, output->length, stream);
        fclose(stream);
        return true;
    }
}

FILE* openFileStream(const File* file, const char* mode) {
    return openPath(toConstPath(file->absolute_path), mode);
}
