
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "compiler/compiler.h"
#include "errors/msgprint.h"
#include "parser/wrapper.h"
#include "rodac/params.h"
#include "text/string.h"
#include "util/debug.h"
#include "version.h"

#ifdef TESTS
#include "tests/test.h"
#include "tests/testrun.h"
#endif

int main(int argc, const char* const* argv) {
    srand(clock() + time(NULL));
    CompilerContext* context = createCompilerContext();
    parseProgramParams(argc, argv, context);
    printAndClearMessages(context, stderr);
    DEBUG_LOG(context, "finished parsing command line arguments");
    if (context->settings.run_kind == COMPILER_RUN_VERSION) {
        printVersionInfo(stderr);
    } else if (context->settings.run_kind == COMPILER_RUN_HELP) {
        printHelpText();
#ifdef TESTS
    } else if (context->settings.run_kind == COMPILER_RUN_TEST) {
        TestManager test_manager;
        initTestManager(&test_manager);
        runTestManager(&test_manager);
        printTestManagerReport(&test_manager, stderr);
        deinitTestManager(&test_manager);
        freeCompilerContext(context);
        return test_manager.counts[TEST_RESULT_FAILED] + test_manager.counts[TEST_RESULT_ERROR] == 0
            ? EXIT_SUCCESS
            : EXIT_FAILURE;
#endif
    } else {
        if (context->files.file_count == 0) {
            addMessageToContext(&context->msgs,
                createMessage(WARNING_CMD_ARGS, copyFromCString("no input files were given"), 0)
            );
        } else {
            runCompilation(context);
        }
    }
    printAndClearMessages(context, stderr);
    if (context->msgs.error_count == 0) {
        DEBUG_LOG(context, "compilation finished without errors");
        freeCompilerContext(context);
        return EXIT_SUCCESS;
    } else {
        DEBUG_LOG(context, "compilation finished with errors");
        freeCompilerContext(context);
        return EXIT_FAILURE;
    }
}
