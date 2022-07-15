
#include <string.h>

#include "tests/assert.h"
#include "util/tmpalloc.h"

DEFINE_TEST(testTempAllocNone, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocSimple1, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1, "Hello world 1");
    char* ptr2 = allocTempBuffer(&alloc, 100);
    strcpy(ptr2, "Hello world 2");
    char* ptr3 = allocTempBuffer(&alloc, 100);
    strcpy(ptr3, "Hello world 3");
    char* ptr4 = allocTempBuffer(&alloc, 100);
    strcpy(ptr4, "Hello world 4");
    ASSERT_STR_EQUAL(ptr4, "Hello world 4");
    freeTempBuffer(&alloc, ptr4);
    ASSERT_STR_EQUAL(ptr3, "Hello world 3");
    freeTempBuffer(&alloc, ptr3);
    ASSERT_STR_EQUAL(ptr2, "Hello world 2");
    freeTempBuffer(&alloc, ptr2);
    ASSERT_STR_EQUAL(ptr1, "Hello world 1");
    freeTempBuffer(&alloc, ptr1);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocSimple2, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1, "Hello world 1");
    char* ptr2 = allocTempBuffer(&alloc, 100);
    strcpy(ptr2, "Hello world 2");
    char* ptr3 = allocTempBuffer(&alloc, 100);
    strcpy(ptr3, "Hello world 3");
    char* ptr4 = allocTempBuffer(&alloc, 100);
    strcpy(ptr4, "Hello world 4");
    ASSERT_STR_EQUAL(ptr1, "Hello world 1");
    freeTempBuffer(&alloc, ptr1);
    ASSERT_STR_EQUAL(ptr2, "Hello world 2");
    freeTempBuffer(&alloc, ptr2);
    ASSERT_STR_EQUAL(ptr3, "Hello world 3");
    freeTempBuffer(&alloc, ptr3);
    ASSERT_STR_EQUAL(ptr4, "Hello world 4");
    freeTempBuffer(&alloc, ptr4);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocSimple3, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1, "Hello world 1");
    char* ptr2 = allocTempBuffer(&alloc, 100);
    strcpy(ptr2, "Hello world 2");
    char* ptr3 = allocTempBuffer(&alloc, 100);
    strcpy(ptr3, "Hello world 3");
    char* ptr4 = allocTempBuffer(&alloc, 100);
    strcpy(ptr4, "Hello world 4");
    ASSERT_STR_EQUAL(ptr1, "Hello world 1");
    freeTempBuffer(&alloc, ptr1);
    ASSERT_STR_EQUAL(ptr3, "Hello world 3");
    freeTempBuffer(&alloc, ptr3);
    ASSERT_STR_EQUAL(ptr4, "Hello world 4");
    freeTempBuffer(&alloc, ptr4);
    ASSERT_STR_EQUAL(ptr2, "Hello world 2");
    freeTempBuffer(&alloc, ptr2);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocDifferentSizes, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1 + 9, "Hello world 1");
    char* ptr2 = allocTempBuffer(&alloc, 1000);
    strcpy(ptr2 + 99, "Hello world 2");
    char* ptr3 = allocTempBuffer(&alloc, 10000);
    strcpy(ptr3 + 999, "Hello world 3");
    char* ptr4 = allocTempBuffer(&alloc, 100000);
    strcpy(ptr4 + 9999, "Hello world 4");
    ASSERT_STR_EQUAL(ptr1 + 9, "Hello world 1");
    freeTempBuffer(&alloc, ptr1);
    ASSERT_STR_EQUAL(ptr3 + 999, "Hello world 3");
    freeTempBuffer(&alloc, ptr3);
    ASSERT_STR_EQUAL(ptr4 + 9999, "Hello world 4");
    freeTempBuffer(&alloc, ptr4);
    ASSERT_STR_EQUAL(ptr2 + 99, "Hello world 2");
    freeTempBuffer(&alloc, ptr2);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocReuse, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1, "Hello world 1");
    ASSERT_STR_EQUAL(ptr1, "Hello world 1");
    freeTempBuffer(&alloc, ptr1);
    char* ptr2 = allocTempBuffer(&alloc, 100);
    ASSERT_EQUAL(ptr1, ptr2);
    strcpy(ptr2, "Hello world 2");
    ASSERT_STR_EQUAL(ptr2, "Hello world 2");
    freeTempBuffer(&alloc, ptr2);
    char* ptr3 = allocTempBuffer(&alloc, 100);
    ASSERT_EQUAL(ptr1, ptr3);
    strcpy(ptr3, "Hello world 3");
    ASSERT_STR_EQUAL(ptr3, "Hello world 3");
    freeTempBuffer(&alloc, ptr3);
    char* ptr4 = allocTempBuffer(&alloc, 100);
    ASSERT_EQUAL(ptr1, ptr4);
    strcpy(ptr4, "Hello world 4");
    ASSERT_STR_EQUAL(ptr4, "Hello world 4");
    freeTempBuffer(&alloc, ptr4);
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocNoFree, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    char* ptr1 = allocTempBuffer(&alloc, 100);
    strcpy(ptr1, "Hello world 1");
    char* ptr2 = allocTempBuffer(&alloc, 100);
    strcpy(ptr2, "Hello world 2");
    char* ptr3 = allocTempBuffer(&alloc, 100);
    strcpy(ptr3, "Hello world 3");
    char* ptr4 = allocTempBuffer(&alloc, 100);
    strcpy(ptr4, "Hello world 4");
    ASSERT_STR_EQUAL(ptr1, "Hello world 1");
    ASSERT_STR_EQUAL(ptr2, "Hello world 2");
    ASSERT_STR_EQUAL(ptr3, "Hello world 3");
    ASSERT_STR_EQUAL(ptr4, "Hello world 4");
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocManySmall, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    int* ptr[100];
    for (size_t i = 0; i < 100; i++) {
        ptr[i] = allocTempBuffer(&alloc, sizeof(int));
        ptr[i][0] = i;
    }
    ASSERT_TRUE(alloc.pages[0]->count > 10);
    for (size_t i = 0; i < 100; i++) {
        ASSERT_EQUAL(ptr[i][0], (int)i);
        freeTempBuffer(&alloc, ptr[i]);
    }
    deinitTempAlloc(&alloc);
})

DEFINE_TEST(testTempAllocManySmallNoFree, "-", {
    TempAlloc alloc;
    initTempAlloc(&alloc);
    for (size_t i = 0; i < 10000; i++) {
        allocTempBuffer(&alloc, i % 16);
    }
    deinitTempAlloc(&alloc);
})

