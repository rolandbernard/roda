
#include "tests/assert.h"
#include "util/sort.h"

DEFINE_TEST(testSwapIntSmall, "-", {
    int ints[2] = { 1, 2 };
    swap(ints, sizeof(int), 0, 1);
    ASSERT_EQUAL(ints[0], 2);
    ASSERT_EQUAL(ints[1], 1);
})

DEFINE_TEST(testSwapIntLarge, "-", {
    int ints[200];
    for (size_t i = 0; i < 200; i++) {
        ints[i] = i;
    }
    ints[10] = 12;
    ints[100] = 42;
    swap(ints, sizeof(int), 10, 100);
    ASSERT_EQUAL(ints[10], 42);
    ASSERT_EQUAL(ints[100], 12);
    for (size_t i = 0; i < 200; i++) {
        if (i != 10 && i != 100) {
            ASSERT_EQUAL(ints[i], (int)i);
        }
    }
})

typedef struct {
    int a;
    double b;
} TestStruct;

DEFINE_TEST(testSwapStructSmall, "-", {
    TestStruct vals[2] = {
        { 1, 0.5 },
        { 2, 2.5 },
    };
    swap(vals, sizeof(TestStruct), 0, 1);
    ASSERT_EQUAL(vals[0].a, 2);
    ASSERT_EQUAL(vals[0].b, 2.5);
    ASSERT_EQUAL(vals[1].a, 1);
    ASSERT_EQUAL(vals[1].b, 0.5);
})

DEFINE_TEST(testSwapStructLarge, "-", {
    TestStruct vals[200];
    for (size_t i = 0; i < 200; i++) {
        vals[i].a = i;
        vals[i].b = 1.0 / i;
    }
    vals[10].a = 12;
    vals[100].a = 42;
    swap(vals, sizeof(TestStruct), 10, 100);
    ASSERT_EQUAL(vals[10].a, 42);
    ASSERT_EQUAL(vals[10].b, 0.01);
    ASSERT_EQUAL(vals[100].a, 12);
    ASSERT_EQUAL(vals[100].b, 0.1);
    for (size_t i = 0; i < 200; i++) {
        if (i != 10 && i != 100) {
            ASSERT_EQUAL(vals[i].a, (int)i);
            ASSERT_EQUAL(vals[i].b, 1.0 / i);
        }
    }
})

void swapInts(size_t idx_a, size_t idx_b, void* ctx) {
    swap(ctx, sizeof(int), idx_a, idx_b);
}

bool compareInts(size_t idx_a, size_t idx_b, void* ctx) {
    int* arr = (int*)ctx;
    return arr[idx_a] <= arr[idx_b];
}

DEFINE_TEST(testSortIntSmall, "-", {
    int ints[2] = { 2, 1 };
    heapSort(2, swapInts, compareInts, ints);
    ASSERT_EQUAL(ints[0], 1);
    ASSERT_EQUAL(ints[1], 2);
})

DEFINE_TEST(testSortIntLarge, "-", {
    int ints[200];
    for (size_t i = 0; i < 200; i++) {
        ints[i] = (13 * i) % 200;
    }
    heapSort(200, swapInts, compareInts, ints);
    for (size_t i = 0; i < 200; i++) {
        ASSERT_EQUAL(ints[i], (int)i);
    }
})

void swapStructs(size_t idx_a, size_t idx_b, void* ctx) {
    swap(ctx, sizeof(TestStruct), idx_a, idx_b);
}

bool compareStructs(size_t idx_a, size_t idx_b, void* ctx) {
    TestStruct* arr = (TestStruct*)ctx;
    return arr[idx_a].a <= arr[idx_b].a;
}

DEFINE_TEST(testSortStructSmall, "-", {
    TestStruct vals[2] = {
        { 2, 2.5 },
        { 1, 0.5 },
    };
    heapSort(2, swapStructs, compareStructs, vals);
    ASSERT_EQUAL(vals[0].a, 1);
    ASSERT_EQUAL(vals[0].b, 0.5);
    ASSERT_EQUAL(vals[1].a, 2);
    ASSERT_EQUAL(vals[1].b, 2.5);
})

DEFINE_TEST(testSortStructLarge, "-", {
    TestStruct vals[200];
    for (size_t i = 0; i < 200; i++) {
        vals[i].a = (13 * i) % 200;
        vals[i].b = 1.0 / vals[i].a;
    }
    heapSort(200, swapStructs, compareStructs, vals);
    for (size_t i = 0; i < 200; i++) {
        ASSERT_EQUAL(vals[i].a, (int)i);
        ASSERT_EQUAL(vals[i].b, 1.0 / i);
    }
})

