#ifndef _RODA_ANALYSIS_VARBUILD_H_
#define _RODA_ANALYSIS_VARBUILD_H_

#include "compiler/context.h"

void runSymbolResolution(CompilerContext* context);

void runControlFlowReferenceResolution(CompilerContext* context);

void runConstantValueEvaluation(CompilerContext* context);

#endif
