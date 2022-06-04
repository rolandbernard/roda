
#include <stdlib.h>

#include "files/fileset.h"

#include "util/alloc.h"
#include "util/hash.h"

#define INITIAL_CAPACITY 32

void initFileSet(FileSet* fileset) {
    fileset->files = NULL;
    fileset->file_count = 0;
    fileset->file_capacity = 0;
}

static bool isIndexValid(const FileSet* table, size_t idx) {
    return table->files[idx] != NULL;
}

void deinitFileSet(FileSet* fileset) {
    for (size_t i = 0; i < fileset->file_capacity; i++) {
        if (isIndexValid(fileset, i)) {
            freeFile(fileset->files[i]);
        }
    }
    FREE(fileset->files);
}

static bool continueSearch(const FileSet* table, size_t idx, ConstPath key) {
    return table->files[idx] != NULL && compareStrings(tocnstr(table->files[idx]->absolute_path), key) != 0;
}

static size_t findIndexHashTable(const FileSet* table, ConstPath key) {
    size_t idx = hashString(key) % table->file_capacity;
    while (continueSearch(table, idx, key)) {
        idx = (idx + 1) % table->file_capacity;
    }
    return idx;
}

static void rebuildHashTable(FileSet* table, size_t size) {
    FileSet new;
    new.file_capacity = size;
    new.files = ZALLOC(File*, size);
    for (size_t i = 0; i < table->file_capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, tocnstr(table->files[i]->absolute_path));
            new.files[idx] = table->files[i];
        }
    }
    FREE(table->files);
    table->files = new.files;
    table->file_capacity = new.file_capacity;
}

static void tryResizingHashTable(FileSet* table) {
    if (table->file_capacity == 0 || table->file_capacity < table->file_count * 2) {
        rebuildHashTable(table, (table->file_capacity == 0 ? INITIAL_CAPACITY : 3 * table->file_capacity / 2));
    }
}

File* searchFileInSet(const FileSet* fileset, ConstPath absolute_path) {
    size_t idx = findIndexHashTable(fileset, absolute_path);
    if (isIndexValid(fileset, idx)) {
        return fileset->files[idx];
    } else {
        return NULL;
    }
}

File* createFileInSet(FileSet* fileset, ConstPath relative_or_absolute_path) {
    tryResizingHashTable(fileset);
    File* file = createFile(relative_or_absolute_path);
    size_t idx = findIndexHashTable(fileset, toConstPath(file->absolute_path));
    if (isIndexValid(fileset, idx)) {
        freeFile(file);
        return fileset->files[idx];
    } else {
        fileset->files[idx] = file;
        fileset->file_count++;
        return file;
    }
}
