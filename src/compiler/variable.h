#ifndef _ANALYSIS_VARIABLE_H_
#define _ANALYSIS_VARIABLE_H_

#include "files/file.h"
#include "text/string.h"

typedef struct {
    ConstString name;
    Span def_loc;
    // TODO: 
} Variable;

Variable* createVariable(ConstString name, Span def_loc);

void freeVariable(Variable* var);

#endif
