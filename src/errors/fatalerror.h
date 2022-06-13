#ifndef _FATAL_ERROR_H_
#define _FATAL_ERROR_H_

#include <stdnoreturn.h>

#include "text/format.h"
#include "text/string.h"
#include "util/macro.h"

#ifdef DEBUG
#define ASSERT(COND, ...) {                                                                                         \
    if (!(COND)) {                                                                                                  \
        fatalError(str("compiler assertion failed, " __FILE__ ":" STRINGIFY(__LINE__) ", " #COND __VA_ARGS__ ));    \
    }                                                                                                               \
}
#else
#define ASSERT(COND, ...)
#endif

#define UNREACHABLE(...) {                                                                                   \
    fatalError(str("compiler reached unreachable state, " __FILE__ ":" STRINGIFY(__LINE__) __VA_ARGS__ ));   \
}

noreturn void fatalError(ConstString message);

#endif
