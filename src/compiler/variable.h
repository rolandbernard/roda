#ifndef _RODA_ANALYSIS_VARIABLE_H_
#define _RODA_ANALYSIS_VARIABLE_H_

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
    struct AstVar* def;         \
    struct AstVar** refs;       \
    size_t ref_count;           \
    size_t ref_capacity;        \
    void* codegen;

typedef struct {
    SYMBOL_ENTRY_BASE
} SymbolEntry;

typedef struct {
    SYMBOL_ENTRY_BASE
    Type* type;
    struct AstNode* type_reasoning;
} SymbolVariable;

typedef struct SymbolType {
    SYMBOL_ENTRY_BASE
    Type* type;
} SymbolType;

void freeSymbolEntry(SymbolEntry* var);

SymbolVariable* createVariableSymbol(Symbol name, struct AstVar* def);

SymbolType* createTypeSymbol(Symbol name, struct AstVar* def);

SymbolType* createTypeSymbolWithType(Symbol name, struct AstVar* def, Type* type);

void addSymbolReference(SymbolEntry* entry, struct AstVar* var);

#endif
