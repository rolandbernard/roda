#ifndef _RODA_FATAL_ERROR_H_
#define _RODA_FATAL_ERROR_H_

#include <stdnoreturn.h>

#include "text/format.h"
#include "text/string.h"
#include "util/macro.h"

#ifdef DEBUG
#define ASSERT(COND) {                                                                              \
    if (!(COND)) {                                                                                  \
        fatalError(str(__FILE__ ":" XSTRINGIFY(__LINE__) ": compiler assertion failed: " #COND ));  \
    }                                                                                               \
}

#define UNREACHABLE(MSG) {                                                                              \
    fatalError(str(__FILE__ ":" XSTRINGIFY(__LINE__) ": compiler reached unreachable state: " MSG ));   \
}
#else
#define ASSERT(COND)

#define UNREACHABLE(MSG) {                                  \
    fatalError(str("compiler reached unreachable state"));  \
}
#endif

#ifndef noreturn
#define noreturn
#endif

noreturn void fatalError(ConstString message);

#endif
