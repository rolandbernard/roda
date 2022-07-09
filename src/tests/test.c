
#include <string.h>

#include "util/alloc.h"
#include "tests/testrun.h"

#include "tests/test.h"

GlobalTestRegistry global_test_registry;

#define INITIAL_CAPACITY 8

void globallyRegisterTest(GlobalTestRegistry* reg, TestAddTestFunction add_function) {
    if (reg->count == reg->capacity) {
        reg->capacity = reg->capacity == 0 ? INITIAL_CAPACITY : 3 * reg->capacity / 2;
        reg->functions = REALLOC(TestAddTestFunction, reg->functions, reg->capacity);
    }
    reg->functions[reg->count] = add_function;
    reg->count++;
}

void initTestManager(TestManager* manager) {
    manager->tests = NULL;
    for (size_t i = 0; i < global_test_registry.count; i++) {
        global_test_registry.functions[i](manager);
    }
}

void deinitTestManager(TestManager* manager) {
    while (manager->tests != NULL) {
        TestCase* next = manager->tests->next;
        if (manager->tests->deinit != NULL) {
            manager->tests->deinit(manager->tests);
        }
        FREE(manager->tests);
        manager->tests = next;
    }
}

void addTestToManager(
    TestManager* manager, const char* name, const char* desc, TestCaseFunction function,
    void* udata, TestCaseDeinitFunction deinit
) {
    TestCase* test_case = NEW(TestCase);
    test_case->next = manager->tests;
    test_case->manager = manager;
    test_case->name = name;
    test_case->desc = desc;
    test_case->function = function;
    test_case->udata = udata;
    test_case->deinit = deinit;
    memset(&test_case->result, 0, sizeof(TestResult));
    manager->tests = test_case;
}

void runTestManager(TestManager* manager) {
    TestCase* test = manager->tests;
    while (test != NULL) {
        runTestCase(test);
        test = test->next;
    }
}

static const char* getStatusName(TestResultStatus status) {
    switch (status) {
        case TEST_RESULT_UNDONE:
            return "undone";
        case TEST_RESULT_SUCCESS:
            return "success";
        case TEST_RESULT_FAILED:
            return "failure";
        case TEST_RESULT_ERROR:
            return "error";
    }
    return "unknown";
}

void printTestManagerReport(TestManager* manager, FILE* file) {
    TestCase* test = manager->tests;
    while (test != NULL) {
        fprintf(file, "%s: %s\n", getStatusName(test->result.status), test->name);
        test = test->next;
    }
}

