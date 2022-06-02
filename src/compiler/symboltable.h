#ifndef _ANALYSIS_SYMBOLS_H_
#define _ANALYSIS_SYMBOLS_H_

#include <stddef.h>

#include "compiler/variable.h"
#include "text/string.h"

typedef struct SymbolTable {
    struct SymbolTable* parent;
    SymbolEntry** hashed;
    size_t count;
    size_t capacity;
} SymbolTable;

void initSymbolTable(SymbolTable* self, SymbolTable* parent);

void deinitSymbolTable(SymbolTable* self);

void addSymbolToTable(SymbolTable* self, SymbolEntry* var);

SymbolEntry* findImmediateEntryInTable(SymbolTable* self, Symbol name, SymbolEntryKind kind);

SymbolEntry* findEntryInTable(SymbolTable* self, Symbol name, SymbolEntryKind kind);

#endif
