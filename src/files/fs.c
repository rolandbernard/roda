
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "errors/fatalerror.h"
#include "text/format.h"
#include "util/alloc.h"

#include "files/fs.h"

Path getWorkingDirectory() {
    size_t capacity = 32;
    char* str = ALLOC(char, capacity);
    char* ret;
    while ((ret = getcwd(str, capacity)) == NULL && errno == ERANGE) {
        capacity *= 2;
        str = REALLOC(char, str, capacity);
    }
    if (ret == NULL) {
        return createEmptyString();
    } else {
        return inlineReducePath(resizeStringData(createFromCString(ret)));
    }
}

Path getAbsolutePath(ConstPath path) {
    if (isAbsolutePath(path)) {
        return createPath(path);
    } else {
        Path cwd = getWorkingDirectory();
        Path ret = joinPaths(toConstPath(cwd), path);
        freePath(cwd);
        return ret;
    }
}

Path getRelativePath(ConstPath path) {
    if (isRelativePath(path)) {
        return createPath(path);
    } else {
        Path cwd = getWorkingDirectory();
        Path ret = getPathFromTo(toConstPath(cwd), path);
        freePath(cwd);
        return ret;
    }
}

bool existsPath(ConstPath path) {
    if (access(toCString(path), F_OK) == 0) {
        return true;
    } else {
        return false;
    }
}

void removePath(ConstPath path) {
    remove(toCString(path));
}

FILE* openPath(ConstPath path, const char* flags) {
    return fopen(toCString(path), flags);
}

Path getTemporaryFilePathInDir(const char* dir, const char* extension) {
    for (size_t i = 0; i < 10; i++) {
        size_t random = rand();
        Path path = createFormattedString("%s/.rodac_%08x.%s", dir, random, extension);
        FILE* file = fopen(cstr(path), "wx");
        if (file != NULL) {
            fclose(file);
            return path;
        }
        freePath(path);
    }
    return createString(NULL, 0);
}

Path getTemporaryFilePath(const char* extension) {
    Path path = getTemporaryFilePathInDir("/tmp", extension);
    if (path.data != NULL) {
        return path;
    }
    path = getTemporaryFilePathInDir(".", extension);
    if (path.data != NULL) {
        return path;
    }
    fatalError(str("failed to open temporary file"));
}


