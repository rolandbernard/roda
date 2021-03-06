#ifndef _RODAC_VERSION_H_
#define _RODAC_VERSION_H_

#include <stdio.h>

#include "util/macro.h"

#define PROGRAM_NAME "rodac"
#define PROGRAM_LONG "roda compiler"

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

#define VERSION_STRING XSTRINGIFY(VERSION_MAJOR) "." XSTRINGIFY(VERSION_MINOR) "." XSTRINGIFY(VERSION_PATCH)

#ifdef GIT_HEAD
#define VERSION_STRING_BUILD VERSION_STRING "+" GIT_HEAD
#else
#define VERSION_STRING_BUILD VERSION_STRING
#endif

void printVersionInfo(FILE* out);

#endif
