
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "files/path.h"

#include "util/alloc.h"

Path inlineReducePath(Path path) {
    size_t read_pos = 0;
    size_t insert_pos = 0;
    size_t segments = 0; // Holds the number of reducable segments e.g: /../seg1/seg2/ ('..' is not reducable)
    bool is_absolute = false;
    if (path.data[read_pos] == '/') {
        read_pos++;
        insert_pos++;
        is_absolute = true;
    }
    while (read_pos < path.length) {
        // Skip unnecessary '/' characters
        while (read_pos < path.length && path.data[read_pos] == '/') {
            read_pos++;
        }
        // Find end of segment
        size_t next_slash = read_pos;
        while (next_slash < path.length && path.data[next_slash] != '/') {
            next_slash++;
        }
        if (next_slash == read_pos + 1 && path.data[read_pos] == '.') {
            // 'seg1/./' is equal to 'seg1/' 
            read_pos += 2;
        } else if (next_slash == read_pos + 2 && path.data[read_pos] == '.' && path.data[read_pos + 1] == '.') {
            if (segments > 0) {
                // Remove the last segment e.g. '/seg1/seg2' -> '/seg1'
                insert_pos--;
                while(insert_pos > 0 && path.data[insert_pos - 1] != '/') {
                    insert_pos--;
                }
                read_pos += 3;
                segments--;
            } else {
                if (is_absolute) {
                    // '/../' is equal to '/'
                    read_pos += 3;
                } else {
                    // Write the segment (but don't increment the number of reducable segments)
                    while (read_pos <= next_slash && read_pos < path.length) {
                        path.data[insert_pos] = path.data[read_pos];
                        read_pos++;
                        insert_pos++;
                    }
                }
            }
        } else {
            // Write the segment
            while (read_pos <= next_slash && read_pos < path.length) {
                path.data[insert_pos] = path.data[read_pos];
                read_pos++;
                insert_pos++;
            }
            segments++;
        }
    }
    // Remove any trailing '/'
    if (insert_pos > 1 && path.data[insert_pos - 1] == '/') {
        insert_pos--;
    }
    path.length = insert_pos;
    return path;
}

Path reducePath(ConstPath path) {
    Path ret = copyString(path);
    return inlineReducePath(ret);
}

Path joinPaths(ConstPath path1, ConstPath path2) {
    return inlineReducePath(concatNStrings(3, path1, createConstString("/", 1), path2));
}

Path concatPaths(ConstPath path1, ConstPath path2) {
    return inlineReducePath(concatStrings(path1, path2));
}

ConstString getFilename(ConstPath path) {
    size_t end = path.length;
    while (end > 0 && path.data[end - 1] == '/') {
        end--;
    }
    size_t start = end;
    while (start > 0 && path.data[start - 1] != '/') {
        start--;
    }
    return createConstString(path.data + start, end - start);
}

ConstString getExtention(ConstPath path) {
    return getStringAfterChar(getFilename(path), '.');
}

ConstPath getParentDirectory(ConstPath path) {
    size_t end = path.length;
    while (end > 0 && path.data[end -1] == '/') {
        end--;
    }
    while (end > 0 && path.data[end - 1] != '/') {
        end--;
    }
    while (end > 0 && path.data[end - 1] == '/') {
        end--;
    }
    return createConstString(path.data, end);
}

Path getWorkingDirectory() {
    size_t capacity = 32;
    char* str = ALLOC(char, capacity);
    char* ret;
    while ((ret = getcwd(str, capacity)) == NULL && errno == ERANGE) {
        capacity *= 2;
        str = REALLOC(char, str, capacity);
    }
    if (ret == NULL) {
        return createString(NULL, 0);
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

Path getPathFromTo(ConstPath from, ConstPath to) {
    // Reducing the path to get a consistent format (i.e. no '.', no duplicate-'/', all '..' are at the beginning)
    Path reduced_from = reducePath(from);
    Path reduced_to = reducePath(to);
    // Maximum e.g. from = 'a/b/c', to = 'g' -> '../../../g'
    size_t max_length = (reduced_from.length + 1) * 3 / 2 + reduced_to.length;
    char* data = ALLOC(char, max_length);
    size_t read_from_pos = 0;
    size_t read_to_pos = 0;
    size_t insert_pos = 0;
    // Skip all segments that are shared
    while (read_from_pos < reduced_from.length && read_to_pos < reduced_to.length) {
        // Skip unnecessary '/' characters
        while (read_from_pos < reduced_from.length && reduced_from.data[read_from_pos] == '/') {
            read_from_pos++;
        }
        while (read_to_pos < reduced_to.length && reduced_to.data[read_to_pos] == '/') {
            read_to_pos++;
        }
        // Find end of segments
        size_t next_from_slash = read_from_pos;
        while (next_from_slash < reduced_from.length && reduced_from.data[next_from_slash] != '/') {
            next_from_slash++;
        }
        size_t next_to_slash = read_to_pos;
        while (next_to_slash < reduced_to.length && reduced_to.data[next_to_slash] != '/') {
            next_to_slash++;
        }
        // Compare the segments
        if (
            next_from_slash - read_from_pos == next_to_slash - read_to_pos
            && strncmp(reduced_from.data + read_from_pos, reduced_to.data + read_to_pos, next_from_slash - read_from_pos) == 0
        ) {
            read_from_pos = next_from_slash;
            read_to_pos = next_to_slash;
        } else {
            break;
        }
    }
    // Insert '..' for the segments in from
    while (read_from_pos < reduced_from.length) {
        // Skip unnecessary '/' characters
        while (read_from_pos < reduced_from.length && reduced_from.data[read_from_pos] == '/') {
            read_from_pos++;
        }
        // Find end of segments
        size_t next_slash = read_from_pos;
        while (next_slash < reduced_from.length && reduced_from.data[next_slash] != '/') {
            next_slash++;
        }
        if (next_slash == read_from_pos + 2 && reduced_from.data[read_from_pos] == '.' && reduced_from.data[read_from_pos + 1] == '.') {
            // We can't know the correct directory, but if it is a absolute path this works (since '/../' is equal to '/')
            read_from_pos += 3;
        } else {
            // Write the '..'
            data[insert_pos] = '.';
            data[insert_pos + 1] = '.';
            data[insert_pos + 2] = '/';
            insert_pos += 3;
            read_from_pos = next_slash;
        }
    }
    memcpy(data + insert_pos, reduced_to.data + read_to_pos, reduced_to.length - read_to_pos);
    freePath(reduced_from);
    freePath(reduced_to);
    return inlineReducePath(createString(data, insert_pos + reduced_to.length - read_to_pos));
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

Path createPath(ConstString path) {
    return inlineReducePath(copyString(path));
}

Path createPathFromCString(const char* path) {
    return inlineReducePath(copyString(createFromConstCString((char*)path)));
}

void freePath(Path path) {
    freeString(path);
}

bool isAbsolutePath(ConstPath path) {
    return path.length > 0 && path.data[0] == '/';
}

bool isRelativePath(ConstPath path) {
    return !isAbsolutePath(path);
}

ConstPath toConstPath(Path path) {
    return toConstString(path);
}

Path toNonConstPath(ConstPath path) {
    return toNonConstString(path);
}

bool comparePaths(ConstPath a, ConstPath b) {
    return compareStrings(a, b) == 0;
}

Path copyPath(ConstPath path) {
    return copyString(path);
}
