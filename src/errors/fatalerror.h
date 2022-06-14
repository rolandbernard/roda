#ifndef _FATAL_ERROR_H_
#define _FATAL_ERROR_H_

#include <stdnoreturn.h>

#include "text/format.h"
#include "text/string.h"
#include "util/macro.h"

#ifdef DEBUG
#define ASSERT(COND, ...) {                                                                                                         \
    if (!(COND)) {                                                                                                                  \
        fatalError(str(__FILE__ ":" XSTRINGIFY(__LINE__) ": compiler assertion failed: " #COND __VA_OPT__(": ") __VA_ARGS__ ));     \
    }                                                                                                                               \
}
#else
#define ASSERT(COND, ...)
#endif

#define UNREACHABLE(...) {                                                                                                      \
    fatalError(str(__FILE__ ":" XSTRINGIFY(__LINE__) ": compiler reached unreachable state"  __VA_OPT__(": ") __VA_ARGS__ ));   \
}

noreturn void fatalError(ConstString message);

#endif
