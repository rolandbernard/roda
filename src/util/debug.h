#ifndef _UTIL_DEBUG_
#define _UTIL_DEBUG_

#ifdef DEBUG
#include "util/console.h"

#define DEBUG_DO(CXT, KIND, ACTION)                         \
    if (((CXT)->settings.compiler_debug & (KIND)) != 0) {   \
        ACTION                                              \
    }

#define DEBUG_LOG(CXT, STRING)                                                                              \
    DEBUG_DO(CXT, COMPILER_DEBUG_LOG, {                                                                     \
        fprintf(stderr, CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_BLACK) "debug log: " STRING CONSOLE_SGR() "\n");  \
    })
#else
#define DEBUG_DO(CXT, KIND, ACTION)
#define DEBUG_LOG(CXT, ...)
#endif

#endif
