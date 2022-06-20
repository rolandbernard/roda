#ifndef _RODA_TEXT_SYMBOL_H_
#define _RODA_TEXT_SYMBOL_H_

#include "text/string.h"

typedef struct {
    String* symbols;
    size_t count;
    size_t capacity;
} SymbolContext;

typedef const char* Symbol;

void initSymbolContext(SymbolContext* context);

void deinitSymbolContext(SymbolContext* context);

Symbol getSymbol(SymbolContext* context, ConstString str);

#endif
