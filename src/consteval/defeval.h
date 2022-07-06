#ifndef _RODA_CONSTEVAL_DEFEVAL_H_
#define _RODA_CONSTEVAL_DEFEVAL_H_

#include "compiler/context.h"

void runGlobalInitEvaluation(CompilerContext* context);

void runConstantValueEvaluation(CompilerContext* context);

#endif
