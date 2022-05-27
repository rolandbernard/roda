
#include <stdbool.h>

#include "util/alloc.h"

#include "analysis/symbols.h"

#define INITIAL_CAPACITY 32

void initSymbolTable(SymbolTable* self, SymbolTable* parent) {
    self = NEW(SymbolTable);
    self->parent = parent;
    self->names = NULL;
    self->vars = NULL;
    self->count = 0;
    self->capacity = 0;
}

void deinitSymbolTable(SymbolTable* self) {
    FREE(self->names);
    FREE(self->vars);
}

static bool isIndexValid(SymbolTable* table, size_t idx) {
    return table->vars[idx] != NULL;
}

static bool continueSearch(SymbolTable* table, size_t idx, ConstString key) {
    return table->vars[idx] != NULL && compareStrings(table->names[idx], key) != 0;
}

static size_t findIndexHashTable(SymbolTable* table, ConstString key) {
    size_t idx = hashString(key) % table->capacity;
    while (continueSearch(table, idx, key)) {
        idx = (idx + 1) % table->capacity;
    }
    return idx;
}

Variable* findSymbolInTable(SymbolTable* self, ConstString name) {
    if (self->count != 0) {
        size_t idx = findIndexHashTable(self, name);
        if (isIndexValid(self, idx)) {
            return self->vars[idx];
        }
    }
    if (self->parent == NULL) {
        return NULL;
    } else {
        return findSymbolInTable(self->parent, name);
    }
}

static void rebuildHashTable(SymbolTable* table, size_t size) {
    SymbolTable new;
    new.capacity = size;
    new.count = 0;
    new.names = ALLOC(ConstString, size);
    new.vars = ALLOC(Variable*, size);
    for (size_t i = 0; i < size; i++) {
        new.vars[i] = NULL;
    }
    for (size_t i = 0; i < table->capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, table->names[i]);
            new.names[idx] = table->names[i];
            new.vars[idx] = table->vars[i];
        }
    }
    FREE(table->names);
    table->names = new.names;
    FREE(table->vars);
    table->vars = new.vars;
    table->capacity = new.capacity;
}

static void tryResizingHashTable(SymbolTable* table) {
    if (table->capacity == 0 || table->capacity < table->count * 2) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    } else if (table->capacity / 2 > INITIAL_CAPACITY && table->capacity > table->count * 4) {
        rebuildHashTable(table, table->capacity / 2);
    }
}

void addSymbolToTable(SymbolTable* self, ConstString name, Variable* var) {
    tryResizingHashTable(self);
    size_t idx = findIndexHashTable(self, name);
    if (isIndexValid(self, idx)) {
        self->vars[idx] = var;
    } else {
        self->names[idx] = name;
        self->vars[idx] = var;
        self->count++;
    }
}

