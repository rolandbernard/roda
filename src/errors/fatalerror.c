
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG
#include <execinfo.h>
#endif

#include "errors/msgkind.h"
#include "util/alloc.h"
#include "util/console.h"

#include "errors/fatalerror.h"

noreturn void fatalError(ConstString message) {
    bool istty = isATerminal(stderr);
    if (istty) {
        fputs(CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_RED), stderr);
    }
    ConstString error_name = getMessageCategoryName(MESSAGE_FATAL_ERROR);
    fwrite(error_name.data, sizeof(char), error_name.length, stderr);
    if (istty) {
        fputs(CONSOLE_SGR() CONSOLE_SGR(CONSOLE_SGR_BOLD), stderr);
    }
    fputs(": ", stderr);
    fwrite(message.data, sizeof(char), message.length, stderr);
    if (istty) {
        fputs(CONSOLE_SGR(), stderr);
    }
    fputc('\n', stderr);
#ifdef DEBUG
    void* address[10];
    int size = backtrace(address, 10);
    char** symbols = backtrace_symbols(address, size);
    if (symbols != NULL) {
        for (int i = 0; i < size; i++) {
            printf ("    #%i in %s\n", i, symbols[i]);
        }
    }
    FREE(symbols);
#endif
    exit(EXIT_FAILURE);
}
