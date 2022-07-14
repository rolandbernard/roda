
#include "tests/assert.h"
#include "files/path.h"

DEFINE_TEST(testCreatePathAbsolute, "-", {
    Path path = createPath(str("/test/1/2/3"));
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testCreatePathRelative, "-", {
    Path path = createPath(str("test/1/2/3"));
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testCreatePathAbsoluteReducing, "-", {
    Path path = createPath(str("//..//test/0/../1/2/3/./4/.."));
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testCreatePathRelativeReducing, "-", {
    Path path = createPath(str(".//.//test///1/2/3"));
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testCreatePathFromCStringAbsolute, "-", {
    Path path = createPathFromCString("/test/1/2/3");
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testCreatePathFromCStringRelative, "-", {
    Path path = createPathFromCString("test/1/2/3");
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testCreatePathFromCStringAbsoluteReducing, "-", {
    Path path = createPathFromCString("//..//test/0/../1/2/3/./4/..");
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testCreatePathFromCStringRelativeReducing, "-", {
    Path path = createPathFromCString(".//.//test///1/2/3");
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testToConstPath, "-", {
    Path path = createPathFromCString("test/1/2/3");
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ConstPath const_path = toConstPath(path);
    ASSERT_EQUAL(path.data, const_path.data);
    ASSERT_EQUAL(path.length, const_path.length);
    freePath(path);
})

DEFINE_TEST(testToNonConstPath, "-", {
    Path path0 = createPathFromCString("test/1/2/3");
    ConstPath const_path = toConstPath(path0);
    Path path = toNonConstPath(const_path);
    ASSERT_EQUAL(path.data, const_path.data);
    ASSERT_EQUAL(path.length, const_path.length);
    freePath(path);
})

DEFINE_TEST(testJoinPaths, "-", {
    Path path0 = createPathFromCString("test");
    Path path1 = createPathFromCString("1/2/3");
    Path path = joinPaths(toConstPath(path0), toConstPath(path1));
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testJoinPathsReduce, "-", {
    Path path0 = createPathFromCString("test");
    Path path1 = createPathFromCString("../1/2/3");
    Path path = joinPaths(toConstPath(path0), toConstPath(path1));
    ASSERT_STR_EQUAL(path.data, "1/2/3");
    ASSERT_EQUAL(path.length, 5);
    freePath(path);
})

DEFINE_TEST(testConcatPaths, "-", {
    Path path0 = createPathFromCString("test");
    Path path1 = createPathFromCString("1/2/3");
    Path path = concatPaths(toConstPath(path0), toConstPath(path1));
    ASSERT_STR_EQUAL(path.data, "test1/2/3");
    ASSERT_EQUAL(path.length, 9);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathDupSep, "-", {
    Path path0 = copyFromCString("test////1///2///3");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathEndSep, "-", {
    Path path0 = copyFromCString("test/1/2/3////");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathParent, "-", {
    Path path0 = copyFromCString("test/1/2/3/../../../1/2/3");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathBeginParent, "-", {
    Path path0 = copyFromCString("test/1/2/3/../../../../../test/1/2/3");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "../test/1/2/3");
    ASSERT_EQUAL(path.length, 13);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathThis, "-", {
    Path path0 = copyFromCString("./test/./1/./2/././3/./.");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "test/1/2/3");
    ASSERT_EQUAL(path.length, 10);
    freePath(path);
})

DEFINE_TEST(testInlineReducePathBeginParentAbsolute, "-", {
    Path path0 = copyFromCString("/test/1/2/3/../../../../../../test/1/2/3");
    Path path = inlineReducePath(path0);
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testReducePath, "-", {
    ConstPath path0 = str("//..//test/0/../1/2/3/./4/..");
    Path path = reducePath(path0);
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testGetFilename1, "-", {
    ConstPath path0 = str("/test/1/2/3/filename.txt");
    ConstPath path = getFilename(path0);
    ASSERT_STR_EQUAL(path.data, "filename.txt");
    ASSERT_EQUAL(path.length, 12);
})

DEFINE_TEST(testGetExtention1, "-", {
    ConstPath path0 = str("/test/1/2/3/filename.txt");
    ConstPath path = getExtention(path0);
    ASSERT_STR_EQUAL(path.data, "txt");
    ASSERT_EQUAL(path.length, 3);
})

DEFINE_TEST(testGetFilename2, "-", {
    ConstPath path0 = str("/test/1/2/3/filename");
    ConstString name = getFilename(path0);
    ASSERT_STR_EQUAL(name.data, "filename");
    ASSERT_EQUAL(name.length, 8);
})

DEFINE_TEST(testGetExtention2, "-", {
    ConstPath path0 = str("/test/1/2/3/filename");
    ConstString ext = getExtention(path0);
    ASSERT_STR_EQUAL(ext.data, "filename");
    ASSERT_EQUAL(ext.length, 8);
})

DEFINE_TEST(testParentDirectory, "-", {
    ConstPath path0 = str("/test/1/2/3/filename");
    Path path = copyPath(getParentDirectory(path0));
    ASSERT_STR_EQUAL(path.data, "/test/1/2/3");
    ASSERT_EQUAL(path.length, 11);
    freePath(path);
})

DEFINE_TEST(testGetPathFromTo, "-", {
    ConstPath path0 = str("/test/1/2/3");
    ConstPath path1 = str("/test/1/a/b");
    Path path = getPathFromTo(path0, path1);
    ASSERT_STR_EQUAL(path.data, "../../a/b");
    ASSERT_EQUAL(path.length, 9);
    freePath(path);
})

DEFINE_TEST(testGetPathFromToUnknown1, "-", {
    ConstPath path0 = str("/test/1/2/3");
    ConstPath path1 = str("a/b");
    Path path = getPathFromTo(path0, path1);
    ASSERT_STR_EQUAL(path.data, "../../../../a/b");
    ASSERT_EQUAL(path.length, 15);
    freePath(path);
})

DEFINE_TEST(testGetPathFromToUnknown2, "-", {
    ConstPath path0 = str("/test/1/2/3");
    ConstPath path1 = str("test/a/b");
    Path path = getPathFromTo(path0, path1);
    ASSERT_STR_EQUAL(path.data, "../../../a/b");
    ASSERT_EQUAL(path.length, 12);
    freePath(path);
})

DEFINE_TEST(testGetPathFromToRelative, "-", {
    ConstPath path0 = str("a/b/1/2/3");
    ConstPath path1 = str("a/b/c/d");
    Path path = getPathFromTo(path0, path1);
    ASSERT_STR_EQUAL(path.data, "../../../c/d");
    ASSERT_EQUAL(path.length, 12);
    freePath(path);
})

DEFINE_TEST(testIsAbsolutePath, "-", {
    ConstPath path0 = str("/a/b/c");
    ConstPath path1 = str("a/b/c");
    ASSERT_TRUE(isAbsolutePath(path0));
    ASSERT_FALSE(isAbsolutePath(path1));
})

DEFINE_TEST(testIsRelativePath, "-", {
    ConstPath path0 = str("/a/b/c");
    ConstPath path1 = str("a/b/c");
    ASSERT_FALSE(isRelativePath(path0));
    ASSERT_TRUE(isRelativePath(path1));
})

DEFINE_TEST(testComparePaths, "-", {
    ConstPath path0 = str("/a/b/c");
    ConstPath path1 = str("/a/b/c");
    ConstPath path2 = str("/a/b");
    ASSERT_TRUE(comparePaths(path0, path1));
    ASSERT_FALSE(comparePaths(path0, path2));
    ASSERT_FALSE(comparePaths(path1, path2));
})

