
#include <stdbool.h>

#include "util/alloc.h"
#include "util/hash.h"

#include "compiler/symboltable.h"

#define INITIAL_CAPACITY 32

void initSymbolTable(SymbolTable* self, SymbolTable* parent) {
    self->parent = parent;
    self->hashed = NULL;
    self->count = 0;
    self->capacity = 0;
}

void deinitSymbolTable(SymbolTable* self) {
    for (size_t i = 0; i < self->capacity; i++) {
        if (self->hashed[i] != NULL) {
            freeSymbolEntry(self->hashed[i]);
        }
    }
    FREE(self->hashed);
}

static bool isIndexValid(SymbolTable* table, size_t idx) {
    return table->hashed[idx] != NULL;
}

static bool continueSearch(SymbolTable* table, size_t idx, Symbol key, SymbolEntryKind kind) {
    return table->hashed[idx] != NULL && (table->hashed[idx]->name != key || table->hashed[idx]->kind != kind);
}

static size_t findIndexHashTable(SymbolTable* table, Symbol key, SymbolEntryKind kind) {
    size_t idx = hashCombine(hashInt((size_t)key), hashInt(kind)) % table->capacity;
    while (continueSearch(table, idx, key, kind)) {
        idx = (idx + 1) % table->capacity;
    }
    return idx;
}

static void rebuildHashTable(SymbolTable* table, size_t size) {
    SymbolTable new;
    new.capacity = size;
    new.hashed = ALLOC(SymbolEntry*, size);
    for (size_t i = 0; i < size; i++) {
        new.hashed[i] = NULL;
    }
    for (size_t i = 0; i < table->capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, table->hashed[i]->name, table->hashed[i]->kind);
            new.hashed[idx] = table->hashed[i];
        }
    }
    FREE(table->hashed);
    table->hashed = new.hashed;
    table->capacity = new.capacity;
}

static void tryResizingHashTable(SymbolTable* table) {
    if (table->capacity == 0 || table->capacity < table->count * 2) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    }
}

void addSymbolToTable(SymbolTable* self, SymbolEntry* var) {
    tryResizingHashTable(self);
    size_t idx = findIndexHashTable(self, var->name, var->kind);
    if (isIndexValid(self, idx)) {
        self->hashed[idx] = var;
    } else {
        self->hashed[idx] = var;
        self->count++;
    }
}

SymbolEntry* findImmediateEntryInTable(SymbolTable* self, Symbol name, SymbolEntryKind kind) {
    if (self->count != 0) {
        size_t idx = findIndexHashTable(self, name, kind);
        if (isIndexValid(self, idx)) {
            return self->hashed[idx];
        }
    }
    return NULL;
}

SymbolEntry* findEntryInTable(SymbolTable* self, Symbol name, SymbolEntryKind kind) {
    SymbolEntry* ret = findImmediateEntryInTable(self, name, kind);
    if (ret != NULL) {
        return ret;
    } else if (self->parent == NULL) {
        return NULL;
    } else {
        return findEntryInTable(self->parent, name, kind);
    }
}

