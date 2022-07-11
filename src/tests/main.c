
#include <stdlib.h>

#include "tests/test.h"
#include "tests/testrun.h"

int main() {
    TestManager test_manager;
    initTestManager(&test_manager);
    runTestManager(&test_manager);
    printTestManagerReport(&test_manager, stderr);
    deinitTestManager(&test_manager);
    return test_manager.counts[TEST_RESULT_FAILED] + test_manager.counts[TEST_RESULT_ERROR] == 0
        ? EXIT_SUCCESS
        : EXIT_FAILURE;
}
