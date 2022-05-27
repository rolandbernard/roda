#ifndef _ANALYSIS_SYMBOLS_H_
#define _ANALYSIS_SYMBOLS_H_

#include <stddef.h>

#include "text/string.h"
#include "analysis/variable.h"

typedef struct SymbolTable {
    struct SymbolTable* parent;
    ConstString* names;
    Variable** vars;
    size_t count;
    size_t capacity;
} SymbolTable;

void initSymbolTable(SymbolTable* self, SymbolTable* parent);

void deinitSymbolTable(SymbolTable* self);

Variable* findSymbolInTable(SymbolTable* self, ConstString name);

void addSymbolToTable(SymbolTable* self, ConstString name, Variable* var);

#endif
