
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>
#include <setjmp.h>
#include <poll.h>

#include "util/alloc.h"
#include "util/console.h"

#include "tests/testrun.h"

static TestCase* global_test_case = NULL;
static int test_result_pipe = 0;
static jmp_buf test_jmp;

static void writeOutTestCaseResult() {
    write(test_result_pipe, &global_test_case->result, sizeof(TestResult));
}

static void collectChilds() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (global_test_case != NULL) {
            RunningTestCase* running = global_test_case->manager->running_tests;
            for (size_t i = 0; i < global_test_case->manager->jobs; i++) {
                if (running[i].pid == pid) {
                    running[i].exit = status;
                    running[i].pid = 0;
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
}

static void killChilds() {
    if (global_test_case != NULL) {
        RunningTestCase* running = global_test_case->manager->running_tests;
        for (size_t i = 0; i < global_test_case->manager->jobs; i++) {
            if (running[i].pid != 0 && (!global_test_case->manager->isolated || running[i].status == TEST_RUNNING_RUNNING)) {
                kill(running[i].pid, SIGKILL);
            }
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
    global_test_case = test_case;
    test_case->result.desc = "test case not finished";
    job->test_case = test_case;
    job->status = TEST_RUNNING_RUNNING;
    if (test_case->manager->isolated) {
        for (size_t i = 0; i < 4; i++) {
            pipe(job->pipes[i]);
        }
        job->pid = fork();
        if (job->pid == 0) {
#ifdef COVERAGE
            atexit((void (*)(void))__llvm_profile_write_file);
            __llvm_profile_initialize_file();
#endif
            test_result_pipe = job->pipes[3][1];
            atexit(writeOutTestCaseResult);
            dup2(job->pipes[0][0], fileno(stdin));
            dup2(job->pipes[1][1], fileno(stdout));
            dup2(job->pipes[2][1], fileno(stderr));
            test_case->function(test_case);
            test_case->result.status = TEST_RESULT_SUCCESS;
            test_case->result.desc = "test case finished successfully";
            exit(0);
        }
    } else {
        if (job->pid == 0) {
            for (size_t i = 0; i < 5; i++) {
                pipe(job->pipes[i]);
            }
            job->pid = fork();
            if (job->pid == 0) {
#ifdef COVERAGE
                atexit((void (*)(void))__llvm_profile_write_file);
                __llvm_profile_initialize_file();
#endif
                test_result_pipe = job->pipes[3][1];
                atexit(writeOutTestCaseResult);
                dup2(job->pipes[0][0], fileno(stdin));
                dup2(job->pipes[1][1], fileno(stdout));
                dup2(job->pipes[2][1], fileno(stderr));
                while (read(job->pipes[4][0], &global_test_case, sizeof(TestCase*)) == sizeof(TestCase*)) {
                    if (global_test_case == NULL) {
                        exit(0);
                    }
                    if (setjmp(test_jmp) == 0) {
                        global_test_case->function(test_case);
                        global_test_case->result.status = TEST_RESULT_SUCCESS;
                        global_test_case->result.desc = "test case finished successfully";
                    }
                    writeOutTestCaseResult();
                }
                exit(1);
            }
        }
        for (size_t i = 0; i < 4; i++) {
            struct pollfd pfd;
            pfd.fd = job->pipes[i][0];
            pfd.events = POLLIN;
            while (poll(&pfd, 1, 0) == 1) {
                char tmp[512];
                read(job->pipes[i][0], tmp, 512);
            }
        }
        job->exit = 0;
        write(job->pipes[4][1], &test_case, sizeof(TestCase*));
    }
}

static char* readStringFromFd(int fd) {
    int length = 0;
    ioctl(fd, FIONREAD, &length); 
    if (length == 0) {
        return NULL;
    } else {
        char* buffer = (char*)malloc(length + 1);
        if (length > 0) {
            read(fd, buffer, length);
        }
        buffer[length] = 0;
        return buffer;
    }
}

static void stopRunningTestCase(TestManager* manager, RunningTestCase* job) {
    struct pollfd pfd;
    pfd.fd = job->pipes[3][0];
    pfd.events = POLLIN;
    size_t result_read = 0;
    if (poll(&pfd, 1, 0) == 1) {
        result_read = read(job->pipes[3][0], &job->test_case->result, sizeof(TestResult));
    }
    job->test_case->result.out_stderr = readStringFromFd(job->pipes[2][0]);
    if (result_read != sizeof(TestResult) || job->exit != 0 || job->test_case->result.out_stderr != NULL) {
        if (job->test_case->result.status == TEST_RESULT_UNDONE) {
            job->test_case->result.status = TEST_RESULT_ERROR;
            job->test_case->result.desc = WIFEXITED(job->exit)
                ? "test exited with error code"
                : WIFSIGNALED(job->exit)
                    ? "test exited by signal"
                    : "unable to read result from worker process";
        }
        if (!manager->isolated && job->pid != 0) {
            int pid = job->pid;
            job->pid = 0;
            kill(pid, SIGKILL);
        }
    }
    if (job->pid == 0) {
        for (size_t i = 0; i < 5; i++) {
            close(job->pipes[i][0]);
            close(job->pipes[i][1]);
        }
    }
}

typedef void (*SignalHandler)(int);

const char* progress_indicators[] = { "|", "/", "-", "\\" };

#define ARRAY_LEN(A) (sizeof(A) / sizeof((A)[0]))

void runTestManager(TestManager* manager) {
    struct pollfd to_poll[manager->jobs];
    size_t step = 0;
    bool progress = isatty(fileno(stderr));
    SignalHandler oldSigChldHandler = signal(SIGCHLD, signalHandler);
    SignalHandler oldSigIntHandler = signal(SIGINT, signalHandler);
    SignalHandler oldSigTermHandler = signal(SIGTERM, signalHandler);
    SignalHandler oldSigHupHandler = signal(SIGHUP, signalHandler);
    size_t running_tests = 0;
    TestCase* test_case = manager->tests;
    manager->running_tests = ALLOC(RunningTestCase, manager->jobs);
    for (size_t i = 0; i < manager->jobs; i++) {
        manager->running_tests[i].status = TEST_RUNNING_IDLE;
        manager->running_tests[i].pid = 0;
    }
    struct timespec sleep = { .tv_sec = 0, .tv_nsec = 100000000 };
    struct timeval last;
    gettimeofday(&last, NULL);
    while (test_case != NULL || running_tests != 0) {
        size_t num_to_poll = 0;
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
                    if (test_case == NULL && !manager->isolated && run->pid != 0) {
                        write(run->pipes[4][1], &test_case, sizeof(TestCase*));
                    }
                    break;
                case TEST_RUNNING_RUNNING:
                    if (!manager->isolated) {
                        struct pollfd pfd;
                        pfd.fd = run->pipes[3][0];
                        pfd.events = POLLIN;
                        if (poll(&pfd, 1, 0) == 1) {
                            run->status = TEST_RUNNING_EXITED;
                        } else {
                            to_poll[num_to_poll].fd = run->pipes[3][0];
                            to_poll[num_to_poll].events = POLLIN | POLLHUP | POLLERR;
                            num_to_poll++;
                        }
                    }
                    break;
                case TEST_RUNNING_EXITED:
                    manager->counts[run->test_case->result.status]--;
                    stopRunningTestCase(manager, run);
                    manager->counts[run->test_case->result.status]++;
                    running_tests--;
                    run->test_case = NULL;
                    run->status = TEST_RUNNING_IDLE;
                    changed = true;
                    break;
            }
        }
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
        if (!changed) {
            if (manager->isolated) {
                nanosleep(&sleep, NULL);
            } else {
                poll(to_poll, num_to_poll, 100);
            }
        }
    }
    if (progress && step != 0) {
        fprintf(stderr, CONSOLE_CUU(%i) CONSOLE_ED(), TEST_RESULT_STATUS_COUNT);
    }
    if (!manager->isolated) {
        nanosleep(&sleep, NULL);
        killChilds();
        collectChilds();
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
    if (global_test_case->manager->isolated) {
        exit(0);
    } else {
        longjmp(test_jmp, 1);
        exit(1);
    }
}

