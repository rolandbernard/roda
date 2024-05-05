#ifndef _RODA_ANALYSIS_SYMBOLS_H_
#define _RODA_ANALYSIS_SYMBOLS_H_

#include <stddef.h>

#include "compiler/variable.h"

typedef struct SymbolTable {
    struct SymbolTable* parent;
    SymbolEntry* symbols;
    SymbolEntry** hashed;
    size_t count;
    size_t capacity;
} SymbolTable;

void initSymbolTable(SymbolTable* self, SymbolTable* parent);

void deinitSymbolTable(SymbolTable* self);

void addSymbolToTable(SymbolTable* self, SymbolEntry* var);

SymbolEntry* findImmediateEntryInTable(const SymbolTable* self, Symbol name, SymbolEntryKind kind);

SymbolEntry* findEntryInTable(const SymbolTable* self, Symbol name, SymbolEntryKind kind);

#endif
