#ifndef _RODA_ANALYSIS_VARIABLE_H_
#define _RODA_ANALYSIS_VARIABLE_H_

#include "types/types.h"
#include "const/value.h"
#include "text/symbol.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_TYPE,
} SymbolEntryKind;

#define SYMBOL_ENTRY_BASE       \
    struct SymbolEntry* next;   \
    SymbolEntryKind kind;       \
    Symbol name;                \
    struct AstVar* def;         \
    struct AstVar* refs;        \
    void* codegen;

typedef struct SymbolEntry {
    SYMBOL_ENTRY_BASE
} SymbolEntry;

typedef struct SymbolVariable {
    SYMBOL_ENTRY_BASE
    Type* type;
    ConstValue* value;
    struct SymbolVariable* function;
    bool constant;
    bool evaluated;
} SymbolVariable;

typedef struct SymbolType {
    SYMBOL_ENTRY_BASE
    Type* type;
} SymbolType;

#define FOR_ALL_VAR_REFS(VAR, ACTION) { \
    struct AstVar* ref = VAR->refs;     \
    while (ref != NULL) {               \
        ACTION                          \
        ref = ref->next_ref;            \
    }                                   \
}

void freeSymbolEntry(SymbolEntry* var);

SymbolVariable* createVariableSymbol(Symbol name, struct AstVar* def, bool constant, SymbolVariable* function);

SymbolType* createTypeSymbol(Symbol name, struct AstVar* def);

SymbolType* createTypeSymbolWithType(Symbol name, struct AstVar* def, Type* type);

void addSymbolReference(SymbolEntry* entry, struct AstVar* var);

#endif
