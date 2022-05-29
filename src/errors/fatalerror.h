#ifndef _FATAL_ERROR_H_
#define _FATAL_ERROR_H_

#include <stdnoreturn.h>

#include "text/format.h"
#include "text/string.h"
#include "util/macro.h"

#define ASSERT(COND) {                                                                                  \
    if (!(COND)) {                                                                                      \
        fatalError(str("Compiler assertion failed, " __FILE__ ":" STRINGIFY(__LINE__) ", " #COND ));    \
    }                                                                                                   \
}

noreturn void fatalError(ConstString message);

#endif
