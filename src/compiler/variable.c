
#include "util/alloc.h"

#include "compiler/variable.h"

Variable* createVariable(ConstString name) {
    Variable* var = NEW(Variable);
    var->name = name;
    return var;
}

void freeVariable(Variable* var) {
    FREE(var);
}

