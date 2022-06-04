
#include <stdlib.h>

#include "files/fileset.h"

#include "util/alloc.h"
#include "util/hash.h"

#define INITIAL_CAPACITY 32

void initFileSet(FileSet* fileset) {
    fileset->files = NULL;
    fileset->file_count = 0;
    fileset->file_capacity = 0;
    fileset->hashed = NULL;
    fileset->hashed_capacity = 0;
}

void deinitFileSet(FileSet* fileset) {
    for (size_t i = 0; i < fileset->file_count; i++) {
        freeFile(fileset->files[i]);
    }
    FREE(fileset->files);
    FREE(fileset->hashed);
}

static bool isIndexValid(const FileSet* table, size_t idx) {
    return table->hashed[idx] != NULL;
}

static bool continueSearch(const FileSet* table, size_t idx, ConstPath key) {
    return table->hashed[idx] != NULL && compareStrings(tocnstr(table->hashed[idx]->absolute_path), key) != 0;
}

static size_t findIndexHashTable(const FileSet* table, ConstPath key) {
    size_t idx = hashString(key) % table->hashed_capacity;
    while (continueSearch(table, idx, key)) {
        idx = (idx + 1) % table->hashed_capacity;
    }
    return idx;
}

static void rebuildHashTable(FileSet* table, size_t size) {
    FileSet new;
    new.hashed_capacity = size;
    new.hashed = ZALLOC(File*, size);
    for (size_t i = 0; i < table->hashed_capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, tocnstr(table->hashed[i]->absolute_path));
            new.hashed[idx] = table->hashed[i];
        }
    }
    FREE(table->hashed);
    table->hashed = new.hashed;
    table->hashed_capacity = new.hashed_capacity;
}

static void tryResizingHashTable(FileSet* table) {
    if (table->hashed_capacity == 0 || 2 * table->hashed_capacity < 3 * table->file_count) {
        rebuildHashTable(table, (table->hashed_capacity == 0 ? INITIAL_CAPACITY : 3 * table->hashed_capacity / 2));
    }
}

File* searchFileInSet(const FileSet* fileset, ConstPath absolute_path) {
    size_t idx = findIndexHashTable(fileset, absolute_path);
    if (isIndexValid(fileset, idx)) {
        return fileset->hashed[idx];
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
        return fileset->hashed[idx];
    } else {
        fileset->hashed[idx] = file;
        if (fileset->file_count == fileset->file_capacity) {
            fileset->file_capacity = fileset->file_capacity == 0 ? INITIAL_CAPACITY : 3 * fileset->file_capacity / 2;
            fileset->files = REALLOC(File*, fileset->files, fileset->file_capacity);
        }
        fileset->files[fileset->file_count] = file;
        fileset->file_count++;
        return file;
    }
}
