
#include "util/alloc.h"

#include "compiler/variable.h"

SymbolVariable* createVariableSymbol(Symbol name, const struct AstVar* def) {
    SymbolVariable* sym = NEW(SymbolVariable);
    sym->name = name;
    sym->def = def;
    return sym;
}

SymbolType* createTypeSymbol(Symbol name, const struct AstVar* def) {
    SymbolType* sym = NEW(SymbolType);
    sym->name = name;
    sym->def = def;
    return sym;
}

void freeSymbolEntry(SymbolEntry* var) {
    switch (var->kind) {
        case SYMBOL_VARIABLE:
        case SYMBOL_TYPE:
            break;
    }
    FREE(var);
}

