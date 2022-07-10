#ifndef _TEST_FRAMEWORK_ASSERT_H_
#define _TEST_FRAMEWORK_ASSERT_H_

#include <stdbool.h>
#include <string.h>

#include "tests/test.h"
#include "tests/testrun.h"

#define ASSERT_FAIL(KIND, MSG) \
    raiseTestFailure(KIND, MSG, __FILE__, __LINE__)

#define ASSERT_TRUE(COND) \
    if (!(COND)) { ASSERT_FAIL(TEST_ASSERT_TRUE, "failed assertion: expected " #COND " to be true"); }

#define ASSERT_FALSE(COND) \
    if (COND) { ASSERT_FAIL(TEST_ASSERT_FALSE, "failed assertion: expected " #COND " to be false"); }

#define ASSERT_NULL(COND) \
    if ((COND) != NULL) { ASSERT_FAIL(TEST_ASSERT_NULL, "failed assertion: expected " #COND " to be NULL"); }

#define ASSERT_NOT_NULL(COND) \
    if ((COND) == NULL) { ASSERT_FAIL(TEST_ASSERT_NOT_NULL, "failed assertion: expected " #COND " to be not NULL"); }

#define ASSERT_EQUAL(A, B) \
    if ((A) != (B)) { ASSERT_FAIL(TEST_ASSERT_EQUAL, "failed assertion: expected " #A " to equal " #B); }

#define ASSERT_UNEQUAL(A, B) \
    if ((A) == (B)) { ASSERT_FAIL(TEST_ASSERT_EQUAL, "failed assertion: expected " #A " to not equal " #B); }

#define ASSERT_STR_EQUAL(A, B) \
    if (strcmp(A, B) != 0) { ASSERT_FAIL(TEST_ASSERT_STR_EQUAL, "failed assertion: expected " #A " to equal " #B); }

#define ASSERT_STR_UNEQUAL(A, B) \
    if (strcmp(A, B) == 0) { ASSERT_FAIL(TEST_ASSERT_STR_EQUAL, "failed assertion: expected " #A " to not equal " #B); }

#endif
