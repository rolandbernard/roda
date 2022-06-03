#ifndef _RODAC_PARAMS_H_
#define _RODAC_PARAMS_H_

#include "compiler/context.h"

void printHelpText();

int parseProgramParams(int argc, const char* const* argv, CompilerContext* context);

#endif
