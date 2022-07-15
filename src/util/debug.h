#ifndef _RODA_UTIL_DEBUG_
#define _RODA_UTIL_DEBUG_

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

#define DEBUG_ONLY(SOME) SOME

#define DEBUG_IF_ELSE(SOME, OTHER) SOME
#else
#define DEBUG_DO(CXT, KIND, ACTION)
#define DEBUG_LOG(CXT, STRING)
#define DEBUG_ONLY(SOME)
#define DEBUG_IF_ELSE(SOME, OTHER) OTHER
#endif

#endif
