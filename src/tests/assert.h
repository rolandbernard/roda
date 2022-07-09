#ifndef _TEST_FRAMEWORK_ASSERT_H_
#define _TEST_FRAMEWORK_ASSERT_H_

#include <stdbool.h>

#include "tests/test.h"
#include "tests/testrun.h"

#define ASSERT_FAIL(KIND, MSG)    \
    raiseTestFailure(TEST_RESULT_FAILED, KIND, MSG, __FILE__, __LINE__)

#define ASSERT_TRUE(COND)   \
    if (!(COND)) { ASSERT_FAIL(TEST_ASSERT_TRUE, "Failed assertion" #COND); }

#endif
