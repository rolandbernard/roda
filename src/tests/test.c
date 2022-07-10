
#include <string.h>

#include "util/alloc.h"
#include "util/console.h"
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
    manager->jobs = 12;
    manager->running_tests = NULL;
    for (size_t i = 0; i < TEST_RESULT_STATUS_COUNT; i++) {
        manager->counts[i] = 0;
    }
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

TestCase* addTestToManager(
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
    manager->counts[test_case->result.status]++;
    return test_case;
}

static const char* getStatusName(TestResultStatus status) {
    switch (status) {
        case TEST_RESULT_UNDONE:
            return " undone";
        case TEST_RESULT_SUCCESS:
            return "success";
        case TEST_RESULT_FAILED:
            return "failure";
        case TEST_RESULT_ERROR:
            return "  error";
    }
    return "unknown";
}

static const char* getStatusStyle(TestResultStatus status) {
    switch (status) {
        case TEST_RESULT_UNDONE:
            return CONSOLE_SGR();
        case TEST_RESULT_SUCCESS:
            return CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_GREEN);
        case TEST_RESULT_FAILED:
            return CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_RED);
        case TEST_RESULT_ERROR:
            return CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_RED);
    }
    return CONSOLE_SGR();
}

void printTestManagerReport(TestManager* manager, FILE* file) {
    size_t printed = 0;
    TestCase* test = manager->tests;
    while (test != NULL) {
        if (test->result.status != TEST_RESULT_SUCCESS) {
            fprintf(
                file, "%s%s:" CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD) " %s: %s" CONSOLE_SGR() "\n",
                getStatusStyle(test->result.status), getStatusName(test->result.status), test->name, test->desc
            );
            if (test->result.desc != NULL) {
                fprintf(file, " --> %s:%zi: %s\n", test->result.file, test->result.line, test->result.desc);
            }
            printed++;
        }
        test = test->next;
    }
    if (printed != 0) {
        fprintf(file, "\n");
    }
    printTestManagerProgress(manager, file);
}

void printTestManagerProgress(TestManager* manager, FILE* file) {
    for (size_t i = 0; i < TEST_RESULT_STATUS_COUNT; i++) {
        fprintf(
            file, "%s%s: %zi" CONSOLE_SGR() "\n", getStatusStyle(i), getStatusName(i),
            manager->counts[i]
        );
    }
}

