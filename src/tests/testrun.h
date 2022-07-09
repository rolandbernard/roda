#ifndef _TEST_FRAMEWORK_TESTRUN_H_
#define _TEST_FRAMEWORK_TESTRUN_H_

#include "tests/test.h"

void runTestCase(TestCase* test_case);

void raiseTestFailure(TestResultStatus status, TestAssertKind kind, const char* msg, const char* file, size_t line);

#endif
