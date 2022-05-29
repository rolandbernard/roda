#ifndef _TEXT_SYMBOL_H_
#define _TEXT_SYMBOL_H_

#include "text/string.h"

typedef struct {
    String* symbols;
    size_t* table;
    size_t count;
    size_t capacity;
} SymbolContext;

typedef size_t Symbol;

void initSymbolContext(SymbolContext* context);

void deinitSymbolContext(SymbolContext* context);

ConstString getSymbolName(SymbolContext* context, Symbol symbol);

Symbol getSymbol(SymbolContext* context, ConstString str);

#endif
