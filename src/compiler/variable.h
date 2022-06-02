#ifndef _ANALYSIS_VARIABLE_H_
#define _ANALYSIS_VARIABLE_H_

#include "compiler/types.h"
#include "files/file.h"
#include "text/string.h"
#include "text/symbol.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_TYPE,
} SymbolEntryKind;

#define SYMBOL_ENTRY_BASE       \
    SymbolEntryKind kind;       \
    Symbol name;                \
    const struct AstVar* def;

typedef struct {
    SYMBOL_ENTRY_BASE
} SymbolEntry;

typedef struct {
    SYMBOL_ENTRY_BASE
} SymbolVariable;

typedef struct {
    SYMBOL_ENTRY_BASE
    Type* type;
} SymbolType;

void freeSymbolEntry(SymbolEntry* var);

SymbolVariable* createVariableSymbol(Symbol name, const struct AstVar* def);

SymbolType* createTypeSymbol(Symbol name, const struct AstVar* def);

#endif
