
#include "util/alloc.h"

#include "compiler/variable.h"

static void initSymbolEntry(SymbolEntry* entry, SymbolEntryKind kind, Symbol name, struct AstVar* def) {
    entry->kind = kind;
    entry->name = name;
    entry->def = def;
    entry->refs = NULL;
    entry->ref_count = 0;
    entry->ref_capacity = 0;
}

SymbolVariable* createVariableSymbol(Symbol name, struct AstVar* def) {
    SymbolVariable* sym = NEW(SymbolVariable);
    initSymbolEntry((SymbolEntry*)sym, SYMBOL_VARIABLE, name, def);
    sym->type = NULL;
    sym->constant = false;
    return sym;
}

SymbolType* createTypeSymbol(Symbol name, struct AstVar* def) {
    SymbolType* sym = NEW(SymbolType);
    initSymbolEntry((SymbolEntry*)sym, SYMBOL_TYPE, name, def);
    sym->type = NULL;
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

#define INITIAL_CAPACITY 8

void addSymbolReference(SymbolEntry* entry, struct AstVar* var) {
    if (entry->ref_count == entry->ref_capacity) {
        entry->ref_capacity = entry->ref_capacity == 0 ? INITIAL_CAPACITY : 3 * entry->ref_capacity / 2;
        entry->refs = REALLOC(struct AstVar*, entry->refs, entry->ref_capacity);
    }
    entry->refs[entry->ref_count] = var;
    entry->ref_count++;
}

