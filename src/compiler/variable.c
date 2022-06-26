
#include "ast/ast.h"
#include "util/alloc.h"

#include "compiler/variable.h"

static void initSymbolEntry(SymbolEntry* entry, SymbolEntryKind kind, Symbol name, struct AstVar* def) {
    entry->next = NULL;
    entry->kind = kind;
    entry->name = name;
    entry->def = def;
    entry->refs = NULL;
    entry->codegen = NULL;
}

SymbolVariable* createVariableSymbol(Symbol name, struct AstVar* def) {
    SymbolVariable* sym = NEW(SymbolVariable);
    initSymbolEntry((SymbolEntry*)sym, SYMBOL_VARIABLE, name, def);
    sym->type = NULL;
    sym->type_reasoning = NULL;
    return sym;
}

SymbolType* createTypeSymbol(Symbol name, struct AstVar* def) {
    return createTypeSymbolWithType(name, def, NULL);
}

SymbolType* createTypeSymbolWithType(Symbol name, struct AstVar* def, Type* type) {
    SymbolType* sym = NEW(SymbolType);
    initSymbolEntry((SymbolEntry*)sym, SYMBOL_TYPE, name, def);
    sym->type = type;
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

void addSymbolReference(SymbolEntry* entry, struct AstVar* var) {
    var->next_ref = entry->refs;
    entry->refs = var;
}

