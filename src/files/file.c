
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "files/file.h"
#include "util/alloc.h"

void initFile(File* file, ConstPath file_path) {
    file->original_path = copyPath(file_path);
    file->absolute_path = getAbsolutePath(file_path);
    file->directory = getParentDirectory(toConstPath(file->absolute_path));
    file->extention = getExtention(toConstPath(file->absolute_path));
    file->name = getFilename(toConstPath(file->absolute_path));
}

File* copyFile(File* file) {
    File* ret = ALLOC(File, 1);
    ret->original_path = copyPath(toConstPath(file->original_path));
    ret->absolute_path = copyPath(toConstPath(file->absolute_path));
    ret->directory = getParentDirectory(toConstPath(ret->absolute_path));
    ret->extention = getExtention(toConstPath(ret->absolute_path));
    ret->name = getFilename(toConstPath(ret->absolute_path));
    return ret;
}

File* createFile(ConstPath file_path) {
    File* ret = ALLOC(File, 1);
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

Span createSpan(File* file, int offset, int length) {
    Span ret = {
        .file = file,
        .offset = offset,
        .length = length,
    };
    return ret;
}

Span createSpanFromBounds(File* file, int start, int end) {
    Span ret = {
        .file = file,
        .offset = start,
        .length = end - start,
    };
    return ret;
}

bool isSpanValid(Span span) {
    return span.file != NULL;
}

bool isSpanFileOnly(Span span) {
    return span.file != NULL && (span.offset < 0 || span.length <= 0);
}

int getSpanEndOffset(Span span) {
    return span.offset + span.length;
}

bool loadFileData(File* file, String* output) {
    char path[file->absolute_path.length + 1];
    memcpy(path, file->absolute_path.data, file->absolute_path.length);
    path[file->absolute_path.length] = 0;
    FILE* stream = fopen(path, "r");
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

FILE* openFileStream(File* file, const char* mode) {
    char path[file->absolute_path.length + 1];
    memcpy(path, file->absolute_path.data, file->absolute_path.length);
    path[file->absolute_path.length] = 0;
    FILE* stream = fopen(path, mode);
    return stream;
}
