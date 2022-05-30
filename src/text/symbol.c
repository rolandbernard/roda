
#include <stdbool.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/hash.h"

#include "text/symbol.h"

#define INITIAL_CAPACITY 32

void initSymbolContext(SymbolContext* context) {
    context->symbols = NULL;
    context->capacity = 0;
    context->count = 0;
}

static bool isIndexValid(SymbolContext* table, size_t idx) {
    return table->symbols[idx].data != NULL;
}

void deinitSymbolContext(SymbolContext* context) {
    for (size_t i = 0; i < context->capacity; i++) {
        if (isIndexValid(context, i)) {
            freeString(context->symbols[i]);
        }
    }
    FREE(context->symbols);
}

static bool continueSearch(SymbolContext* table, size_t idx, ConstString key) {
    return table->symbols[idx].data != NULL && compareStrings(tocnstr(table->symbols[idx]), key) != 0;
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
    new.symbols = ALLOC(String, size);
    for (size_t i = 0; i < size; i++) {
        new.symbols[i].data = NULL;
    }
    for (size_t i = 0; i < table->capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, tocnstr(table->symbols[i]));
            new.symbols[idx] = table->symbols[i];
        }
    }
    FREE(table->symbols);
    table->symbols = new.symbols;
    table->capacity = new.capacity;
}

static void tryResizingHashTable(SymbolContext* table) {
    if (table->capacity == 0 || table->capacity < table->count * 2) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    }
}

Symbol getSymbol(SymbolContext* context, ConstString str) {
    tryResizingHashTable(context);
    size_t idx = findIndexHashTable(context, str);
    if (!isIndexValid(context, idx)) {
        context->symbols[idx] = copyString(str);
        context->count++;
    }
    return context->symbols[idx].data;
}

