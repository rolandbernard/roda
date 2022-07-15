
#include <unistd.h>

#include "tests/assert.h"
#include "util/alloc.h"
#include "util/params.h"

PARAM_SPEC_FUNCTION(parameterSpecFunction, char**, {
    size_t i = 0;
    PARAM_USAGE("TEST [options] files...");
    PARAM_FLAG('A', "a-test", {
        context[i++] = strdup("a"); 
    }, "testing option A");
    PARAM_VALUED('B', "b-test", {
        context[i++] = strdup(value); 
    }, false, "=<value>", "testing option B");
    PARAM_STRING_LIST('C', "c-test", {
        context[i++] = strdup(value); 
    }, "=<value>[,...]", "testing option C");
    PARAM_INTEGER('D', "d-test", {
        context[i++] = (char*)(intptr_t)value;
    }, "=<integer>", "testing option D");
    PARAM_DEFAULT({ context[i++] = strdup(value); });
    PARAM_WARNING({ context[i++] = strdup(warning); });
})

DEFINE_TEST(testHelpText, "-", {
    int pipes[2];
    pipe(pipes);
    dup2(pipes[1], 2);
    char text[1024];
    PARAM_PRINT_HELP(parameterSpecFunction, NULL);
    fflush(stderr);
    close(pipes[1]);
    size_t len = read(pipes[0], text, 1024);
    text[len] = 0;
    ASSERT_STR_EQUAL(text,
        "Usage: TEST [options] files...\n"
        "Options:\n"
        "  -A --a-test              testing option A\n"
        "  -B --b-test=<value>      testing option B\n"
        "  -C --c-test=<value>[,...]\n"
        "                           testing option C\n"
        "  -D --d-test=<integer>    testing option D\n"
    );
})

DEFINE_TEST(testFlagParam, "-", {
    const char* argv[] = { "TEST", "-A" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "a");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testValueParamNext, "-", {
    const char* argv[] = { "TEST", "-B", "bb" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_STR_EQUAL(res[0], "bb");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testValueParamFullNext, "-", {
    const char* argv[] = { "TEST", "--b-test", "bb" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_STR_EQUAL(res[0], "bb");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testValueParamEq, "-", {
    const char* argv[] = { "TEST", "-B=bb" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "bb");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testValueParamFullEq, "-", {
    const char* argv[] = { "TEST", "--b-test=bb" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "bb");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testValueParamTag, "-", {
    const char* argv[] = { "TEST", "-Bbb" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "bb");
    ASSERT_NULL(res[1]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testListParamNext, "-", {
    const char* argv[] = { "TEST", "-C", "c,cc" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_STR_EQUAL(res[0], "c");
    ASSERT_STR_EQUAL(res[1], "cc");
    ASSERT_NULL(res[2]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testListParamFullNext, "-", {
    const char* argv[] = { "TEST", "--c-test", "c,cc" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_STR_EQUAL(res[0], "c");
    ASSERT_STR_EQUAL(res[1], "cc");
    ASSERT_NULL(res[2]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testListParamEq, "-", {
    const char* argv[] = { "TEST", "-C=c,cc" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "c");
    ASSERT_STR_EQUAL(res[1], "cc");
    ASSERT_NULL(res[2]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testListParamFullEq, "-", {
    const char* argv[] = { "TEST", "--c-test=c,cc" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "c");
    ASSERT_STR_EQUAL(res[1], "cc");
    ASSERT_NULL(res[2]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testListParamTag, "-", {
    const char* argv[] = { "TEST", "-Cc,cc" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_STR_EQUAL(res[0], "c");
    ASSERT_STR_EQUAL(res[1], "cc");
    ASSERT_NULL(res[2]);
    for (size_t i = 0; res[i] != NULL; i++) {
        FREE(res[i]);
    }
})

DEFINE_TEST(testIntegerParamNext, "-", {
    const char* argv[] = { "TEST", "-D", "42" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_EQUAL(res[0], (char*)42);
    ASSERT_NULL(res[1]);
})

DEFINE_TEST(testIntegerParamFullNext, "-", {
    const char* argv[] = { "TEST", "--d-test", "42" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 3, argv, res);
    ASSERT_EQUAL(res[0], (char*)42);
    ASSERT_NULL(res[1]);
})

DEFINE_TEST(testIntegerParamEq, "-", {
    const char* argv[] = { "TEST", "-D=42" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_EQUAL(res[0], (char*)42);
    ASSERT_NULL(res[1]);
})

DEFINE_TEST(testIntegerParamFullEq, "-", {
    const char* argv[] = { "TEST", "--d-test=42" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_EQUAL(res[0], (char*)42);
    ASSERT_NULL(res[1]);
})

DEFINE_TEST(testIntegerParamTag, "-", {
    const char* argv[] = { "TEST", "-D42" };
    char* res[50];
    memset(res, 0, sizeof(res));
    PARAM_PARSE_ARGS(parameterSpecFunction, 2, argv, res);
    ASSERT_EQUAL(res[0], (char*)42);
    ASSERT_NULL(res[1]);
})

