
#include "util/alloc.h"

#include "compiler/variable.h"

Variable* createVariable(Symbol name, Span def_loc) {
    Variable* var = NEW(Variable);
    var->name = name;
    var->def_loc = def_loc;
    return var;
}

void freeVariable(Variable* var) {
    FREE(var);
}

