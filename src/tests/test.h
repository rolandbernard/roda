#ifndef _TEST_FRAMEWORK_TEST_H_
#define _TEST_FRAMEWORK_TEST_H_

#include <stddef.h>
#include <stdio.h>

struct TestManager;
struct TestCase;

typedef void (*TestAddTestFunction)(struct TestManager* manager);

typedef struct {
    TestAddTestFunction* functions;
    size_t count;
    size_t capacity;
} GlobalTestRegistry;

extern GlobalTestRegistry global_test_registry;

void globallyRegisterTest(GlobalTestRegistry* reg, TestAddTestFunction add_function);

typedef enum {
    TEST_RESULT_UNDONE = 0,
    TEST_RESULT_SUCCESS,
    TEST_RESULT_FAILED,
    TEST_RESULT_ERROR,
} TestResultStatus;

#define TEST_RESULT_STATUS_COUNT (TEST_RESULT_ERROR + 1)

typedef enum {
    TEST_ASSERT_TRUE,
    TEST_ASSERT_NULL,
    TEST_ASSERT_NOT_NULL,
    TEST_ASSERT_EQUAL,
    TEST_ASSERT_UNEQUAL,
} TestAssertKind;

typedef struct {
    TestResultStatus status;
    TestAssertKind failure;
    const char* desc;
    const char* file;
    size_t line;
} TestResult;

typedef void (*TestCaseFunction)(struct TestCase* test);
typedef void (*TestCaseDeinitFunction)(struct TestCase* test);

typedef struct TestCase {
    struct TestCase* next;
    struct TestManager* manager;
    const char* name;
    const char* desc;
    TestCaseFunction function;
    void* udata;
    TestCaseDeinitFunction deinit;
    TestResult result;
} TestCase;

typedef enum {
    TEST_RUNNING_IDLE,
    TEST_RUNNING_RUNNING,
    TEST_RUNNING_EXITED,
} RunningTestCaseStatus;

typedef struct {
    RunningTestCaseStatus status;
    TestCase* test_case;
    int pid;
    int exit;
    int pipes[4][2];
} RunningTestCase;

typedef struct TestManager {
    TestCase* tests;
    size_t jobs;
    RunningTestCase* running_tests;
    size_t counts[TEST_RESULT_STATUS_COUNT];
} TestManager;

void initTestManager(TestManager* manager);

void deinitTestManager(TestManager* manager);

void addTestToManager(
    TestManager* manager, const char* name, const char* desc, TestCaseFunction function,
    void* udata, TestCaseDeinitFunction deinit
);

void printTestManagerReport(TestManager* manager, FILE* file);

#define DEFINE_TEST(NAME, DESC, CODE)                                       \
    static void NAME ## _run (TestCase* test_case) {                        \
        test_case->result.file = __FILE__;                                  \
        test_case->result.line = __LINE__;                                  \
        { CODE }                                                            \
    }                                                                       \
    static void NAME ## _add (TestManager* manager) {                       \
        addTestToManager(manager, DESC, #NAME, NAME ## _run, NULL, NULL);   \
    }                                                                       \
    static void NAME ## _init () __attribute__((constructor));              \
    static void NAME ## _init () {                                          \
        globallyRegisterTest(&global_test_registry, NAME ## _add);          \
    }

#endif
