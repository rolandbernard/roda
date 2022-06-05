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
    struct AstVar* def;   \
    struct AstVar** refs; \
    size_t ref_count;           \
    size_t ref_capacity;

typedef struct {
    SYMBOL_ENTRY_BASE
} SymbolEntry;

typedef struct {
    SYMBOL_ENTRY_BASE
    Type* type;
    bool constant;
} SymbolVariable;

typedef struct SymbolType {
    SYMBOL_ENTRY_BASE
    Type* type;
} SymbolType;

void freeSymbolEntry(SymbolEntry* var);

SymbolVariable* createVariableSymbol(Symbol name, struct AstVar* def);

SymbolType* createTypeSymbol(Symbol name, struct AstVar* def);

void addSymbolReference(SymbolEntry* entry, struct AstVar* var);

#endif
