
#include <stdbool.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/hash.h"

#include "text/symbol.h"

#define INITIAL_CAPACITY 32

void initSymbolContext(SymbolContext* context) {
    context->symbols = NULL;
    context->table = NULL;
    context->capacity = 0;
    context->count = 0;
}

void deinitSymbolContext(SymbolContext* context) {
    for (size_t i = 0; i < context->count; i++) {
        freeString(context->symbols[i]);
    }
    FREE(context->symbols);
    FREE(context->table);
}

static bool isIndexValid(SymbolContext* table, size_t idx) {
    return table->table[idx] != NO_POS;
}

static bool continueSearch(SymbolContext* table, size_t idx, ConstString key) {
    return table->table[idx] != NO_POS && compareStrings(tocnstr(table->symbols[table->table[idx]]), key) != 0;
}

static size_t findIndexHashTable(SymbolContext* table, ConstString key) {
    size_t idx = hashString(key) % table->capacity;
    while (continueSearch(table, idx, key)) {
        idx = (idx + 1) % table->capacity;
    }
    return idx;
}

static void rebuildHashTable(SymbolContext* table, size_t size) {
    SymbolContext new;
    new.capacity = size;
    new.symbols = table->symbols;
    new.table = ALLOC(size_t, size);
    for (size_t i = 0; i < size; i++) {
        new.table[i] = NO_POS;
    }
    for (size_t i = 0; i < table->capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, tocnstr(table->symbols[table->table[i]]));
            new.table[idx] = table->table[i];
        }
    }
    FREE(table->table);
    table->table = new.table;
    table->capacity = new.capacity;
    table->symbols = REALLOC(String, table->symbols, table->capacity);
}

static void tryResizingHashTable(SymbolContext* table) {
    if (table->capacity == 0 || table->capacity < table->count * 2) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    }
}

ConstString getSymbolName(SymbolContext* context, Symbol symbol) {
    ASSERT(symbol < context->count);
    return tocnstr(context->symbols[symbol]);
}

Symbol getSymbol(SymbolContext* context, ConstString str) {
    tryResizingHashTable(context);
    size_t idx = findIndexHashTable(context, str);
    if (!isIndexValid(context, idx)) {
        context->table[idx] = context->count;
        context->symbols[context->count] = copyString(str);
        context->count++;
    }
    return context->table[idx];
}

