
#include "tests/assert.h"
#include "files/fs.h"

DEFINE_TEST(testGetWorkingDirectory, "-", {
    Path path = getWorkingDirectory();
    freePath(path);
})

DEFINE_TEST(testGetAbsolutePath, "-", {
    ConstPath path0 = str("./test");
    Path path = getAbsolutePath(path0);
    freePath(path);
})

DEFINE_TEST(testGetRelativePath, "-", {
    ConstPath path0 = str("test");
    Path path1 = getAbsolutePath(path0);
    Path path2 = getRelativePath(toConstPath(path1));
    fprintf(stderr, "%s ", path0.data);
    fprintf(stderr, "%s ", path1.data);
    fprintf(stderr, "%s ", path2.data);
    ASSERT_STR_EQUAL(path2.data, "test");
    freePath(path1);
    freePath(path2);
})

DEFINE_TEST(testExistsPath, "-", {
    Path path1 = getWorkingDirectory();
    Path path2 = createPathFromCString("some/path/that/does/not/exist");
    ASSERT_TRUE(existsPath(toConstPath(path1)));
    ASSERT_FALSE(existsPath(toConstPath(path2)));
    freePath(path1);
    freePath(path2);
})

DEFINE_TEST(testRemovePath, "-", {
    FILE* file = fopen(".rodac-test-file-remove-path", "w");
    fclose(file);
    Path path = createPathFromCString(".rodac-test-file-remove-path");
    ASSERT_TRUE(existsPath(toConstPath(path)));
    removePath(toConstPath(path));
    ASSERT_FALSE(existsPath(toConstPath(path)));
    freePath(path);
})

DEFINE_TEST(testOpenPath, "-", {
    Path path = createPathFromCString(".rodac-test-file-open-path");
    FILE* file = openPath(toConstPath(path), "w");
    fclose(file);
    ASSERT_TRUE(existsPath(toConstPath(path)));
    removePath(toConstPath(path));
    ASSERT_FALSE(existsPath(toConstPath(path)));
    freePath(path);
})

DEFINE_TEST(testGetTemporaryFilePath, "-", {
    Path path = getTemporaryFilePath("txt");
    ASSERT_TRUE(existsPath(toConstPath(path)));
    ASSERT_STR_EQUAL(getExtention(toConstPath(path)).data, "txt");
    removePath(toConstPath(path));
    ASSERT_FALSE(existsPath(toConstPath(path)));
    freePath(path);
})

