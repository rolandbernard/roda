#ifndef _FATAL_ERROR_H_
#define _FATAL_ERROR_H_

#include <stdnoreturn.h>

#include "text/string.h"

noreturn void fatalError(ConstString message);

#endif
