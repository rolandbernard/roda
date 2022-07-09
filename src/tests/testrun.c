
#include <pthread.h>
#include <stdbool.h>

#include "tests/testrun.h"

static pthread_key_t getTestCaseKey() {
    static bool initialized = false;
    static pthread_key_t key;
    if (!initialized) {
        pthread_key_create(&key, NULL);
    }
    return key;
}

void __asan_on_error(void) {
    raiseTestFailure(TEST_RESULT_ERROR, 0, "address sanitizer error", NULL, 0);
}

static void* testCaseThreadFunction(void* udata) {
    pthread_setspecific(getTestCaseKey(), udata);
    TestCase* test_case = udata;
    test_case->function(test_case);
    return NULL;
}

void runTestCase(TestCase* test_case) {
    test_case->result.status = TEST_RESULT_UNDONE;
    test_case->result.desc = "test case not finished";
    pthread_t thread;
    pthread_create(&thread, NULL, testCaseThreadFunction, test_case);
    pthread_join(thread, NULL);
    if (test_case->result.status == TEST_RESULT_UNDONE) {
        test_case->result.status = TEST_RESULT_SUCCESS;
        test_case->result.desc = "test case finished successfully";
    }
}

void raiseTestFailure(TestResultStatus status, TestAssertKind kind, const char* msg, const char* file, size_t line) {
    TestCase* test_case = pthread_getspecific(getTestCaseKey());
    test_case->result.status = status;
    test_case->result.failure = kind;
    test_case->result.desc = msg;
    test_case->result.file = file;
    test_case->result.line = line;
    pthread_exit(NULL);
}

