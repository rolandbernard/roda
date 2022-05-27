
#include <stdlib.h>

#include "files/fileset.h"

#include "util/alloc.h"

#define INITIAL_FILESET_CAPACITY 32

void initFileSet(FileSet* fileset) {
    fileset->files = NULL;
    fileset->file_count = 0;
    fileset->file_capacity = 0;
}

void deinitFileSet(FileSet* fileset) {
    for (int i = 0; i < fileset->file_count; i++) {
        freeFile(fileset->files[i]);
    }
    FREE(fileset->files);
}

static void extendFileSetCapacity(FileSet* fileset) {
    if (fileset->file_capacity != 0) {
        fileset->file_capacity *= 2;
    } else {
        fileset->file_capacity = INITIAL_FILESET_CAPACITY;
    }
    fileset->files = REALLOC(File*, fileset->files, fileset->file_capacity);
}

void addFileToSet(FileSet* fileset, File* file) {
    if (fileset->file_count == fileset->file_capacity) {
        extendFileSetCapacity(fileset);
    }
    fileset->files[fileset->file_count] = file;
    fileset->file_count++;
}

File* searchFileInSet(FileSet* fileset, ConstPath absolute_path) {
    for (int i = 0; i < fileset->file_count; i++) {
        if (comparePaths(toConstPath(fileset->files[i]->absolute_path), absolute_path)) {
            return fileset->files[i];
        }
    }
    return NULL;
}

File* createFileInSet(FileSet* fileset, ConstPath relative_or_absolute_path) {
    File* file = createFile(relative_or_absolute_path);
    File* existing = searchFileInSet(fileset, toConstPath(file->absolute_path));
    if (existing != NULL) {
        freeFile(file);
        return existing;
    } else {
        addFileToSet(fileset, file);
        return file;
    }
}
