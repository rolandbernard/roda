#ifndef _PATH_H_
#define _PATH_H_

#include <stdbool.h>

#include "text/string.h"

typedef String Path;

typedef ConstString ConstPath;

Path joinPaths(ConstPath path1, ConstPath path2);

Path concatPaths(ConstPath path1, ConstPath path2);

Path reducePath(ConstPath path);

Path inlineReducePath(Path path);

ConstString getFilename(ConstPath path);

ConstString getExtention(ConstPath path);

ConstPath getParentDirectory(ConstPath path);

Path getPathFromTo(ConstPath from, ConstPath to);

Path createPath(ConstString path);

Path createPathFromCString(const char* path);

void freePath(Path path);

bool isAbsolutePath(ConstPath path);

bool isRelativePath(ConstPath path);

ConstPath toConstPath(Path path);

Path toNonConstPath(ConstPath path);

bool comparePaths(ConstPath a, ConstPath b);

Path copyPath(ConstPath path);

#endif
