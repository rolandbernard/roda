
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"

#include "files/file.h"

void initFile(File* file, ConstPath file_path) {
    file->original_path = copyPath(file_path);
    file->absolute_path = getAbsolutePath(file_path);
    file->directory = getParentDirectory(toConstPath(file->absolute_path));
    file->extention = getExtention(toConstPath(file->absolute_path));
    file->name = getFilename(toConstPath(file->absolute_path));
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
}

void freeFile(File* file) {
    deinitFile(file);
    FREE(file);
}

Span invalidSpan() {
    return createSpan(NULL, NO_POS, NO_POS);
}

Span createSpan(const File* file, size_t offset, size_t length) {
    Span ret = {
        .file = file,
        .offset = offset,
        .length = length,
    };
    return ret;
}

Span createSpanFromBounds(const File* file, size_t start, size_t end) {
    Span ret = {
        .file = file,
        .offset = start,
        .length = end - start,
    };
    return ret;
}

Span createSpanWith(Span begin, Span end) {
    ASSERT(begin.file == end.file);
    Span ret = {
        .file = begin.file,
        .offset = begin.offset,
        .length = getSpanEndOffset(end) - begin.offset,
    };
    return ret;
}

bool isSpanValid(Span span) {
    return span.file != NULL;
}

bool isSpanFileOnly(Span span) {
    return span.file != NULL && span.offset == NO_POS;
}

size_t getSpanEndOffset(Span span) {
    return span.offset + span.length;
}

bool loadFileData(const File* file, String* output) {
    FILE* stream = fopen(cstr(file->absolute_path), "r");
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
    return fopen(cstr(file->absolute_path), mode);
}
