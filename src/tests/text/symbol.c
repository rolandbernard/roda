
#include "tests/assert.h"
#include "text/symbol.h"

DEFINE_TEST(testSymbolsSimple, "-", {
    SymbolContext context;
    initSymbolContext(&context);
    deinitSymbolContext(&context);
})

DEFINE_TEST(testSymbolsEqual, "-", {
    SymbolContext context;
    initSymbolContext(&context);
    Symbol a0 = getSymbol(&context, str("Hello World"));
    ASSERT_STR_EQUAL(a0, "Hello World");
    Symbol a1 = getSymbol(&context, str("Hello World"));
    ASSERT_EQUAL(a0, a1);
    Symbol a2 = getSymbol(&context, str("Hello World"));
    ASSERT_EQUAL(a0, a2);
    Symbol a3 = getSymbol(&context, str("Hello World"));
    ASSERT_EQUAL(a0, a3);
    deinitSymbolContext(&context);
})

DEFINE_TEST(testSymbolsUnequal, "-", {
    SymbolContext context;
    initSymbolContext(&context);
    Symbol a0 = getSymbol(&context, str("Hello World 0"));
    ASSERT_STR_EQUAL(a0, "Hello World 0");
    Symbol a1 = getSymbol(&context, str("Hello World 1"));
    ASSERT_STR_EQUAL(a1, "Hello World 1");
    ASSERT_UNEQUAL(a0, a1);
    Symbol a2 = getSymbol(&context, str("Hello World 2"));
    ASSERT_STR_EQUAL(a2, "Hello World 2");
    ASSERT_UNEQUAL(a0, a2);
    ASSERT_UNEQUAL(a1, a2);
    Symbol a3 = getSymbol(&context, str("Hello World 3"));
    ASSERT_STR_EQUAL(a3, "Hello World 3");
    ASSERT_UNEQUAL(a0, a3);
    ASSERT_UNEQUAL(a1, a3);
    ASSERT_UNEQUAL(a2, a3);
    deinitSymbolContext(&context);
})

DEFINE_TEST(testSymbolsManyEqual, "-", {
    SymbolContext context;
    initSymbolContext(&context);
    Symbol a = getSymbol(&context, str("Hello world"));
    for (size_t i = 0; i < 100000; i++) {
        ASSERT_EQUAL(a, getSymbol(&context, str("Hello world")));
    }
    deinitSymbolContext(&context);
})

DEFINE_TEST(testSymbolsManyUnequal, "-", {
    char name[] = "????, Hello world.";
    SymbolContext context;
    initSymbolContext(&context);
    Symbol a = getSymbol(&context, str(name));
    for (size_t i = 0; i < 10000; i++) {
        name[3] = '0' + i % 10;
        name[2] = '0' + i / 10 % 10;
        name[1] = '0' + i / 100 % 10;
        name[0] = '0' + i / 1000 % 10;
        ASSERT_UNEQUAL(a, getSymbol(&context, str(name)));
    }
    deinitSymbolContext(&context);
})

