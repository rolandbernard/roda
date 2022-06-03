#ifndef _RODAC_PARAMS_H_
#define _RODAC_PARAMS_H_

#include "compiler/context.h"

void printHelpText();

void parseProgramParams(int argc, const char* const* argv, CompilerContext* context);

#endif
