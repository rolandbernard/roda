#ifndef _RODA_FILES_FS_H_
#define _RODA_FILES_FS_H_

#include <stdio.h>

#include "files/path.h"

Path getWorkingDirectory();

Path getAbsolutePath(ConstPath path);

Path getRelativePath(ConstPath path);

bool existsPath(ConstPath path);

void removePath(ConstPath path);

FILE* openPath(ConstPath path, const char* flags);

Path getTemporaryFilePath(const char* extension);

#endif
