#ifndef _ANALYSIS_VARIABLE_H_
#define _ANALYSIS_VARIABLE_H_

#include "text/string.h"

typedef struct {
    ConstString name;
    // TODO: 
} Variable;

Variable* createVariable(ConstString name);

void freeVariable(Variable* var);

#endif
