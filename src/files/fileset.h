#ifndef _RODA_FILES_FILESET_H_
#define _RODA_FILES_FILESET_H_

#include "files/file.h"

typedef struct {
    File* files;
    File* files_last;
    size_t file_count;
    File** hashed;
    size_t hashed_capacity;
} FileSet;

void initFileSet(FileSet* fileset);

void deinitFileSet(FileSet* fileset);

File* searchFileInSet(const FileSet* fileset, ConstPath absolute_path);

File* createFileInSet(FileSet* fileset, ConstPath relative_or_absolute_path);

#endif
