
#include <stdbool.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/hash.h"

#include "compiler/types.h"

#define INITIAL_CAPACITY 32

void initTypeContext(TypeContext* cxt) {
    cxt->types = NULL;
    cxt->count = 0;
    cxt->capacity = 0;
}

static bool isIndexValid(TypeContext* table, size_t idx) {
    return table->types[idx] != NULL;
}

void deinitTypeContext(TypeContext* cxt) {
    for (size_t i = 0; i < cxt->capacity; i++) {
        if (isIndexValid(cxt, i)) {
            FREE(cxt->types[i]);
        }
    }
    FREE(cxt->types);
}

static bool areTypesEqual(const Type* a, const Type* b) {
    ASSERT(a != NULL);
    ASSERT(b != NULL);
    if (a->kind != b->kind) {
        return false;
    } else {
        switch (a->kind) {
            case TYPE_VOID:
                return true;
            case TYPE_INT:
            case TYPE_REAL: {
                TypeSizedPrimitive* ta = (TypeSizedPrimitive*)a;
                TypeSizedPrimitive* tb = (TypeSizedPrimitive*)b;
                return ta->size == tb->size;
            }
            case TYPE_POINTER: {
                TypePointer* ta = (TypePointer*)a;
                TypePointer* tb = (TypePointer*)b;
                return ta->base == tb->base;
            }
            case TYPE_ARRAY: {
                TypeArray* ta = (TypeArray*)a;
                TypeArray* tb = (TypeArray*)b;
                return ta->base == tb->base && ta->size == tb->size;
            }
        }
        UNREACHABLE(", unhandled type kind");
    }
}

static size_t hashType(const Type* type) {
    ASSERT(type != NULL);
    switch (type->kind) {
        case TYPE_VOID:
            return hashInt(type->kind);
        case TYPE_INT:
        case TYPE_REAL: {
            TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
            return hashCombine(hashInt(t->kind), hashInt(t->size));
        }
        case TYPE_POINTER: {
            TypePointer* t = (TypePointer*)type;
            return hashCombine(hashInt(t->kind), hashInt((size_t)t->base));
        }
        case TYPE_ARRAY: {
            TypeArray* t = (TypeArray*)type;
            return hashCombine(hashCombine(hashInt(t->kind), hashInt((size_t)t->base)), hashInt(t->size));
        }
    }
    UNREACHABLE(", unhandled type kind");
}

static bool continueSearch(TypeContext* table, size_t idx, const Type* key) {
    return table->types[idx] != NULL && !areTypesEqual(table->types[idx], key);
}

static size_t findIndexHashTable(TypeContext* table, const Type* key) {
    size_t idx = hashType(key) % table->capacity;
    while (continueSearch(table, idx, key)) {
        idx = (idx + 1) % table->capacity;
    }
    return idx;
}

static void rebuildHashTable(TypeContext* table, size_t size) {
    TypeContext new;
    new.capacity = size;
    new.types = ALLOC(Type*, size);
    for (size_t i = 0; i < size; i++) {
        new.types[i] = NULL;
    }
    for (size_t i = 0; i < table->capacity; i++) {
        if (isIndexValid(table, i)) {
            size_t idx = findIndexHashTable(&new, table->types[i]);
            new.types[idx] = table->types[i];
        }
    }
    FREE(table->types);
    table->types = new.types;
    table->capacity = new.capacity;
}

static void tryResizingHashTable(TypeContext* table) {
    if (table->capacity == 0 || table->capacity < table->count * 2) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    }
}

Type* createTypeIfAbsent(TypeContext* context, const Type* type, size_t size) {
    tryResizingHashTable(context);
    size_t idx = findIndexHashTable(context, type);
    if (!isIndexValid(context, idx)) {
        context->types[idx] = checkedAlloc(size);
        memcpy(context->types[idx], type, size);
        context->count++;
    }
    return context->types[idx];
}

Type* createUnsizedPrimitive(TypeContext* cxt, TypeKind kind) {
    Type type = { .kind = kind };
    return createTypeIfAbsent(cxt, &type, sizeof(Type));
}

Type* createSizedPrimitive(TypeContext* cxt, TypeKind kind, size_t size) {
    TypeSizedPrimitive type = { .kind = kind, .size = size };
    return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeSizedPrimitive));
}

Type* createPointer(TypeContext* cxt, Type* base) {
    TypePointer type = { .kind = TYPE_POINTER, .base = base };
    return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypePointer));
}

Type* createArray(TypeContext* cxt, Type* base, size_t size) {
    TypeArray type = { .kind = TYPE_ARRAY, .base = base, .size = size };
    return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeArray));
}

