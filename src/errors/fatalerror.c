
#include <stdio.h>
#include <stdlib.h>

#include "errors/fatalerror.h"

#include "errors/msgkind.h"
#include "util/console.h"

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
    exit(EXIT_FAILURE);
}
