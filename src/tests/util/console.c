
#include "tests/assert.h"
#include "util/console.h"

DEFINE_TEST(testIsNotATerminal, "-", {
    ASSERT_FALSE(isATerminal(stderr));
})

