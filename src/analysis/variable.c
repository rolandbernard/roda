
#include "util/alloc.h"

#include "analysis/variable.h"

Variable* createVariable(ConstString name) {
    Variable* var = NEW(Variable);
    var->name = name;
    return var;
}

void freeVariable(Variable* var) {
    FREE(var);
}

