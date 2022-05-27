#ifndef _FILESET_H_
#define _FILESET_H_

#include "files/file.h"

typedef struct {
    File** files;
    int file_count;
    int file_capacity;
} FileSet;

void initFileSet(FileSet* fileset);

void deinitFileSet(FileSet* fileset);

void addFileToSet(FileSet* fileset, File* file);

File* searchFileInSet(FileSet* fileset, ConstPath absolute_path);

File* createFileInSet(FileSet* fileset, ConstString import_path, ConstPath relative_or_absolute_path);

#endif