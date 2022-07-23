#ifndef _RODA_FATAL_ERROR_H_
#define _RODA_FATAL_ERROR_H_

#include <stddef.h>
#include <stdnoreturn.h>

#include "text/format.h"
#include "text/string.h"
#include "util/macro.h"

#ifndef unreachable
#if defined(__GNUC__)

#define unreachable() (__builtin_unreachable())

#elif defined(_MSC_VER)

#define unreachable() (__assume(false))

#else

static noreturn inline void unreachable_impl() {}

#define unreachable() (unreachable_impl())

#endif
#endif

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

#define UNREACHABLE(MSG) unreachable();
#endif

noreturn void fatalError(ConstString message);

#endif
