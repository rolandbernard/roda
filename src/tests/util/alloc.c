
#include "tests/assert.h"
#include "util/alloc.h"

DEFINE_TEST(testAlloc, "ALLOC allocates N of the given type", {
    size_t* a = ALLOC(size_t, 10);
    for (size_t i = 0; i < 10; i++) {
        a[i] = i;
    }
    for (size_t i = 0; i < 10; i++) {
        ASSERT_EQUAL(a[i], i);
    }
    FREE(a);
})

DEFINE_TEST(testZalloc, "ZALLOC allocates N of the given type initialized to zero", {
    size_t* a = ZALLOC(size_t, 10);
    for (size_t i = 0; i < 10; i++) {
        ASSERT_EQUAL(a[i], 0);
        a[i] = i;
    }
    for (size_t i = 0; i < 10; i++) {
        ASSERT_EQUAL(a[i], i);
    }
    FREE(a);
})

DEFINE_TEST(testNew, "NEW allocates an instance of the given type", {
    size_t* a = NEW(size_t);
    *a = 42;
    ASSERT_EQUAL(*a, 42);
    FREE(a);
})

DEFINE_TEST(testRealloc, "REALLOC resized previously allocated memory", {
    size_t* a = ALLOC(size_t, 10);
    for (size_t i = 0; i < 10; i++) {
        a[i] = i;
    }
    a = REALLOC(size_t, a, 20);
    for (size_t i = 0; i < 10; i++) {
        ASSERT_EQUAL(a[i], i);
        a[10 + i] = 10 + i;
    }
    for (size_t i = 10; i < 20; i++) {
        ASSERT_EQUAL(a[i], i);
    }
    a = REALLOC(size_t, a, 5);
    for (size_t i = 0; i < 5; i++) {
        ASSERT_EQUAL(a[i], i);
    }
    FREE(a);
})

