
#include "tests/assert.h"
#include "files/fileset.h"

DEFINE_TEST(testInitDeinitFileSet, "-", {
    FileSet files;
    initFileSet(&files);
    deinitFileSet(&files);
})

DEFINE_TEST(testCreateFilesInSetDifferent, "-", {
    FileSet files;
    initFileSet(&files);
    File* file0 = createFileInSet(&files, str("tests/.rodac-test-file-0"));
    File* file1 = createFileInSet(&files, str("tests/.rodac-test-file-1"));
    ASSERT_UNEQUAL(file0, file1);
    File* file2 = createFileInSet(&files, str("tests/.rodac-test-file-2"));
    ASSERT_UNEQUAL(file1, file2);
    File* file3 = createFileInSet(&files, str("tests/.rodac-test-file-3"));
    ASSERT_UNEQUAL(file2, file3);
    File* file4 = createFileInSet(&files, str("tests/.rodac-test-file-4"));
    ASSERT_UNEQUAL(file3, file4);
    File* file5 = createFileInSet(&files, str("tests/.rodac-test-file-5"));
    ASSERT_UNEQUAL(file4, file5);
    File* file6 = createFileInSet(&files, str("tests/.rodac-test-file-6"));
    ASSERT_UNEQUAL(file5, file6);
    File* file7 = createFileInSet(&files, str("tests/.rodac-test-file-7"));
    ASSERT_UNEQUAL(file6, file7);
    ASSERT_EQUAL(files.file_count, 8);
    deinitFileSet(&files);
})

DEFINE_TEST(testCreateFilesInSetSame, "-", {
    FileSet files;
    initFileSet(&files);
    File* file0 = createFileInSet(&files, str("tests/.rodac-test-file"));
    for (size_t i = 0; i < 100; i++) {
        ASSERT_EQUAL(file0, createFileInSet(&files, str("tests/.rodac-test-file")));
    }
    ASSERT_EQUAL(files.file_count, 1);
    deinitFileSet(&files);
})

DEFINE_TEST(testSearchFileInSet, "-", {
    FileSet files;
    initFileSet(&files);
    File* file0 = createFileInSet(&files, str("tests/.rodac-test-file-0"));
    createFileInSet(&files, str("tests/.rodac-test-file-1"));
    createFileInSet(&files, str("tests/.rodac-test-file-2"));
    createFileInSet(&files, str("tests/.rodac-test-file-3"));
    File* file4 = createFileInSet(&files, str("tests/.rodac-test-file-4"));
    createFileInSet(&files, str("tests/.rodac-test-file-5"));
    createFileInSet(&files, str("tests/.rodac-test-file-6"));
    File* file7 = createFileInSet(&files, str("tests/.rodac-test-file-7"));
    ASSERT_EQUAL(file0, createFileInSet(&files, str("tests/.rodac-test-file-0")));
    ASSERT_EQUAL(file4, createFileInSet(&files, str("tests/.rodac-test-file-4")));
    ASSERT_EQUAL(file7, createFileInSet(&files, str("tests/.rodac-test-file-7")));
    ASSERT_EQUAL(files.file_count, 8);
    ASSERT_EQUAL(file0, searchFileInSet(&files, str("tests/.rodac-test-file-0")));
    ASSERT_EQUAL(file4, searchFileInSet(&files, str("tests/.rodac-test-file-4")));
    ASSERT_EQUAL(file7, searchFileInSet(&files, str("tests/.rodac-test-file-7")));
    deinitFileSet(&files);
})

