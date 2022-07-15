
#include "tests/assert.h"
#include "files/file.h"
#include "files/fs.h"

DEFINE_TEST(testInitDeinitFile, "-", {
    File file;
    initFile(&file, str("tests/.rodac-test-file.rd"));
    ASSERT_STR_EQUAL(file.original_path.data, "tests/.rodac-test-file.rd");
    ASSERT_STR_EQUAL(file.name.data, ".rodac-test-file.rd");
    ASSERT_STR_EQUAL(file.extention.data, "rd");
    ASSERT_NULL(file.ast);
    ASSERT_NULL(file.next);
    ASSERT_EQUAL(file.type, FILE_RODA);
    deinitFile(&file);
})

DEFINE_TEST(testInitDeinitFileObject, "-", {
    File file;
    initFile(&file, str("tests/.rodac-test-file.o"));
    ASSERT_EQUAL(file.type, FILE_OBJECT);
    deinitFile(&file);
})

DEFINE_TEST(testInitDeinitFileLlvmIr, "-", {
    File file;
    initFile(&file, str("tests/.rodac-test-file.ll"));
    ASSERT_EQUAL(file.type, FILE_LLVM_IR);
    deinitFile(&file);
})

DEFINE_TEST(testInitDeinitFileLlvmBc, "-", {
    File file;
    initFile(&file, str("tests/.rodac-test-file.bc"));
    ASSERT_EQUAL(file.type, FILE_LLVM_BC);
    deinitFile(&file);
})

DEFINE_TEST(testCopyFile, "-", {
    File file0;
    initFile(&file0, str("tests/.rodac-test-file.txt"));
    File* file = copyFile(&file0);
    deinitFile(&file0);
    ASSERT_STR_EQUAL(file->original_path.data, "tests/.rodac-test-file.txt");
    ASSERT_STR_EQUAL(file->name.data, ".rodac-test-file.txt");
    ASSERT_STR_EQUAL(file->extention.data, "txt");
    ASSERT_NULL(file->ast);
    ASSERT_NULL(file->next);
    ASSERT_EQUAL(file->type, FILE_RODA);
    freeFile(file);
})

DEFINE_TEST(testCreateFile, "-", {
    File* file = createFile(str("tests/.rodac-test-file.rd"));
    ASSERT_STR_EQUAL(file->original_path.data, "tests/.rodac-test-file.rd");
    ASSERT_STR_EQUAL(file->name.data, ".rodac-test-file.rd");
    ASSERT_STR_EQUAL(file->extention.data, "rd");
    ASSERT_NULL(file->ast);
    ASSERT_NULL(file->next);
    ASSERT_EQUAL(file->type, FILE_RODA);
    freeFile(file);
})

DEFINE_TEST(testInvalidLocation, "-", {
    Location loc = invalidLocation();
    ASSERT_FALSE(isLocationValid(loc));
    ASSERT_EQUAL(loc.offset, NO_POS);
    ASSERT_EQUAL(loc.column, NO_POS);
    ASSERT_EQUAL(loc.line, NO_POS);
})

DEFINE_TEST(testInvalidSpan, "-", {
    Span span = invalidSpan();
    ASSERT_FALSE(isSpanValid(span));
})

DEFINE_TEST(testCreateFileOnlySpan, "-", {
    File* file = createFile(str(".rodac-test-file.rd"));
    Span span = createFileOnlySpan(file);
    ASSERT_TRUE(isSpanValid(span));
    ASSERT_TRUE(isSpanFileOnly(span));
    ASSERT_FALSE(isSpanWithPosition(span));
    ASSERT_EQUAL(span.file, file);
    freeFile(file);
})

DEFINE_TEST(testCombineSpans, "-", {
    File* file = createFile(str(".rodac-test-file.rd"));
    Span span0 = {
        .file = file,
        .begin = { .offset = 0, .line = 0, .column = 0 },
        .end = { .offset = 10, .line = 2, .column = 0 }
    };
    ASSERT_TRUE(isSpanValid(span0));
    ASSERT_FALSE(isSpanFileOnly(span0));
    ASSERT_TRUE(isSpanWithPosition(span0));
    Span span1 = {
        .file = file,
        .begin = { .offset = 10, .line = 2, .column = 0 },
        .end = { .offset = 20, .line = 2, .column = 10 }
    };
    ASSERT_TRUE(isSpanValid(span1));
    ASSERT_FALSE(isSpanFileOnly(span0));
    ASSERT_TRUE(isSpanWithPosition(span1));
    Span span = combineSpans(span0, span1);
    ASSERT_TRUE(isSpanValid(span));
    ASSERT_FALSE(isSpanFileOnly(span));
    ASSERT_TRUE(isSpanWithPosition(span));
    ASSERT_EQUAL(span.file, file);
    ASSERT_EQUAL(span.begin.offset, 0);
    ASSERT_EQUAL(span.begin.line, 0);
    ASSERT_EQUAL(span.begin.column, 0);
    ASSERT_EQUAL(span.end.offset, 20);
    ASSERT_EQUAL(span.end.line, 2);
    ASSERT_EQUAL(span.end.column, 10);
    freeFile(file);
})

DEFINE_TEST(testGetSpanLength, "-", {
    File* file = createFile(str(".rodac-test-file.rd"));
    Span span = {
        .file = file,
        .begin = { .offset = 0, .line = 0, .column = 0 },
        .end = { .offset = 10, .line = 2, .column = 0 }
    };
    ASSERT_TRUE(isSpanValid(span));
    ASSERT_FALSE(isSpanFileOnly(span));
    ASSERT_TRUE(isSpanWithPosition(span));
    ASSERT_EQUAL(getSpanLength(span), 10);
    freeFile(file);
})

DEFINE_TEST(testGetSpanColumnLength, "-", {
    File* file = createFile(str(".rodac-test-file.rd"));
    Span span = {
        .file = file,
        .begin = { .offset = 0, .line = 0, .column = 0 },
        .end = { .offset = 10, .line = 2, .column = 0 }
    };
    ASSERT_TRUE(isSpanValid(span));
    ASSERT_FALSE(isSpanFileOnly(span));
    ASSERT_TRUE(isSpanWithPosition(span));
    ASSERT_EQUAL(getSpanColumnLength(span), 0);
    freeFile(file);
})

DEFINE_TEST(testAdvanceLocationWith, "-", {
    Location span = { .offset = 0, .line = 0, .column = 0 };
    span = advanceLocationWith(span, "hello\n\nworld\ntesting", 20);
    ASSERT_EQUAL(span.offset, 20);
    ASSERT_EQUAL(span.line, 3);
    ASSERT_EQUAL(span.column, 7);
})

DEFINE_TEST(testOpenFileStream, "-", {
    File* file = createFile(str(".rodac-test-file-open-file-stream.txt"));
    FILE* stream = openFileStream(file, "w");
    fwrite("test", 1, 4, stream);
    fclose(stream);
    removePath(toConstPath(file->absolute_path));
})

DEFINE_TEST(testLoadFileData, "-", {
    File* file = createFile(str(".rodac-test-file-load-file-data.txt"));
    FILE* stream = openFileStream(file, "w");
    fwrite("test", 1, 4, stream);
    fclose(stream);
    String data;
    ASSERT_TRUE(loadFileData(file, &data));
    ASSERT_STR_EQUAL(cstr(data), "test");
    freeString(data);
    removePath(toConstPath(file->absolute_path));
})

