
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

#include "util/alloc.h"
#include "util/console.h"

#include "tests/testrun.h"

static TestCase* global_test_case = NULL;
static int test_result_pipe = 0;

static void writeOutTestCaseResult() {
    write(test_result_pipe, &global_test_case->result, sizeof(TestResult));
}

static TestManager* global_test_manager = NULL;

static void collectChilds() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        RunningTestCase* running = global_test_manager->running_tests;
        for (size_t i = 0; i < global_test_manager->jobs; i++) {
            if (running[i].pid == pid) {
                running[i].exit = status;
                switch (running[i].status) {
                    case TEST_RUNNING_RUNNING:
                        running[i].status = TEST_RUNNING_EXITED;
                        break;
                    default:
                        break;
                }
                break;
            }
        }
    }
}

static void killChilds() {
    RunningTestCase* running = global_test_manager->running_tests;
    for (size_t i = 0; i < global_test_manager->jobs; i++) {
        switch (running[i].status) {
        case TEST_RUNNING_RUNNING:
            kill(-running[i].pid, SIGKILL);
            break;
        default:
            break;
        }
    }
}

static void signalHandler(int signal) {
    if (signal == SIGCHLD) {
        collectChilds();
    } else {
        killChilds();
        collectChilds();
        exit(0);
    }
}

#ifdef COVERAGE
int __llvm_profile_runtime;

extern void __llvm_profile_initialize_file();
extern int __llvm_profile_write_file();
#endif

static void startRunningTestCase(RunningTestCase* job, TestCase* test_case) {
    test_case->result.desc = "test case not finished";
    job->test_case = test_case;
    job->status = TEST_RUNNING_RUNNING;
    for (size_t i = 0; i < 4; i++) {
        pipe(job->pipes[i]);
    }
    job->pid = fork();
    if (job->pid == 0) {
#ifdef COVERAGE
        __llvm_profile_initialize_file();
#endif
        global_test_case = test_case;
        test_result_pipe = job->pipes[3][1];
        atexit(writeOutTestCaseResult);
        dup2(job->pipes[0][0], fileno(stdin));
        dup2(job->pipes[1][1], fileno(stdout));
        dup2(job->pipes[2][1], fileno(stderr));
        test_case->function(test_case);
        test_case->result.status = TEST_RESULT_SUCCESS;
        test_case->result.desc = "test case finished successfully";
#ifdef COVERAGE
        __llvm_profile_write_file();
#endif
        exit(0);
    }
}

static char* readStringFromFd(int fd) {
    int length = 0;
    ioctl(fd, FIONREAD, &length); 
    char* buffer = (char*)malloc(length + 1);
    if (length > 0) {
        read(fd, buffer, length);
    }
    buffer[length] = 0;
    return buffer;
}

static void stopRunningTestCase(RunningTestCase* job) {
    for (size_t i = 0; i < 4; i++) {
        close(job->pipes[i][1]);
    }
    size_t result_read = read(job->pipes[3][0], &job->test_case->result, sizeof(TestResult));
    if (result_read != sizeof(TestResult) || job->exit != 0) {
        job->test_case->result.status = TEST_RESULT_ERROR;
        job->test_case->result.desc = "test exited with error";
        job->test_case->result.out_stderr = readStringFromFd(job->pipes[2][0]);
    }
    for (size_t i = 0; i < 4; i++) {
        close(job->pipes[i][0]);
    }
}

typedef void (*SignalHandler)(int);

const char* progress_indicators[] = { "|", "/", "-", "\\" };

#define ARRAY_LEN(A) (sizeof(A) / sizeof((A)[0]))

void runTestManager(TestManager* manager) {
    size_t step = 0;
    bool progress = isatty(fileno(stderr));
    global_test_manager = manager;
    SignalHandler oldSigChldHandler = signal(SIGCHLD, signalHandler);
    SignalHandler oldSigIntHandler = signal(SIGINT, signalHandler);
    SignalHandler oldSigTermHandler = signal(SIGTERM, signalHandler);
    SignalHandler oldSigHupHandler = signal(SIGHUP, signalHandler);
    size_t running_tests = 0;
    TestCase* test_case = manager->tests;
    manager->running_tests = ALLOC(RunningTestCase, manager->jobs);
    for (size_t i = 0; i < manager->jobs; i++) {
        manager->running_tests[i].status = TEST_RUNNING_IDLE;
    }
    struct timespec sleep = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timeval last;
    gettimeofday(&last, NULL);
    while (test_case != NULL || running_tests != 0) {
        bool changed = false;
        for (size_t i = 0; i < manager->jobs; i++) {
            RunningTestCase* run = &manager->running_tests[i];
            switch (run->status) {
                case TEST_RUNNING_IDLE:
                    if (test_case != NULL) {
                        startRunningTestCase(run, test_case);
                        running_tests++;
                        test_case = test_case->next;
                        changed = true;
                    }
                    break;
                case TEST_RUNNING_RUNNING:
                    break;
                case TEST_RUNNING_EXITED:
                    manager->counts[run->test_case->result.status]--;
                    stopRunningTestCase(run);
                    manager->counts[run->test_case->result.status]++;
                    running_tests--;
                    run->status = TEST_RUNNING_IDLE;
                    changed = true;
                    break;
            }
        }
        if (!changed) {
            if (progress) {
                struct timeval time;
                gettimeofday(&time, NULL);
                if (time.tv_usec / 125000 != last.tv_usec / 125000) {
                    if (step != 0) {
                        fprintf(stderr, CONSOLE_CUU(%i) CONSOLE_ED(), TEST_RESULT_STATUS_COUNT);
                    }
                    printTestManagerProgress(manager, stderr);
                    fprintf(
                        stderr, "     [%s] %zi running\r",
                        progress_indicators[
                            (8 * time.tv_sec + time.tv_usec / 125000)
                            % ARRAY_LEN(progress_indicators)
                        ],
                        running_tests
                    );
                    last = time;
                    step++;
                }
            }
            nanosleep(&sleep, NULL);
        }
    }
    if (progress && step != 0) {
        fprintf(stderr, CONSOLE_CUU(%i) CONSOLE_ED(), TEST_RESULT_STATUS_COUNT);
    }
    FREE(manager->running_tests);
    manager->running_tests = NULL;
    signal(SIGCHLD, oldSigChldHandler);
    signal(SIGINT, oldSigIntHandler);
    signal(SIGTERM, oldSigTermHandler);
    signal(SIGHUP, oldSigHupHandler);
}

void raiseTestFailure(TestAssertKind kind, const char* msg, const char* file, size_t line) {
    global_test_case->result.status = TEST_RESULT_FAILED;
    global_test_case->result.failure = kind;
    global_test_case->result.desc = msg;
    global_test_case->result.file = file;
    global_test_case->result.line = line;
    exit(0);
}

