
#include <stdbool.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "text/format.h"
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

static void freeType(Type* type) {
    if (type != NULL) {
        switch (type->kind) {
            case TYPE_ERROR:
            case TYPE_NEVER:
            case TYPE_VOID:
            case TYPE_INT:
            case TYPE_UINT:
            case TYPE_REAL:
            case TYPE_POINTER:
            case TYPE_ARRAY:
                break;
            case TYPE_FUNCTION: {
                TypeFunction* t = (TypeFunction*)type;
                FREE(t->arguments);
                break;
            }
        }
    }
    FREE(type);
}

void deinitTypeContext(TypeContext* cxt) {
    for (size_t i = 0; i < cxt->capacity; i++) {
        if (isIndexValid(cxt, i)) {
            freeType(cxt->types[i]);
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
            case TYPE_ERROR:
            case TYPE_NEVER:
            case TYPE_VOID:
                return true;
            case TYPE_INT:
            case TYPE_UINT:
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
            case TYPE_FUNCTION: {
                TypeFunction* ta = (TypeFunction*)a;
                TypeFunction* tb = (TypeFunction*)b;
                if (ta->arg_count != tb->arg_count || ta->ret_type != tb->ret_type) {
                    return false;
                } else {
                    for (size_t i = 0; i < ta->arg_count; i++) {
                        if (ta->arguments[i] != tb->arguments[i]) {
                            return false;
                        }
                    }
                    return true;
                }
            }
        }
        UNREACHABLE(", unhandled type kind");
    }
}

static size_t hashType(const Type* type) {
    ASSERT(type != NULL);
    switch (type->kind) {
        case TYPE_ERROR:
        case TYPE_NEVER:
        case TYPE_VOID:
            return hashInt(type->kind);
        case TYPE_INT:
        case TYPE_UINT:
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
        case TYPE_FUNCTION: {
            TypeFunction* t = (TypeFunction*)type;
            size_t hash = hashCombine(hashCombine(hashInt(t->kind), hashInt((size_t)t->ret_type)), hashInt(t->arg_count));
            for (size_t i = 0; i < t->arg_count; i++) {
                hash = hashCombine(hash, hashInt((size_t)t->arguments[i]));
            }
            return hash;
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

TypeSizedPrimitive* createSizedPrimitive(TypeContext* cxt, TypeKind kind, size_t size) {
    TypeSizedPrimitive type = { .kind = kind, .size = size };
    return (TypeSizedPrimitive*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeSizedPrimitive));
}

TypePointer* createPointer(TypeContext* cxt, Type* base) {
    TypePointer type = { .kind = TYPE_POINTER, .base = base };
    return (TypePointer*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypePointer));
}

TypeArray* createArray(TypeContext* cxt, Type* base, size_t size) {
    TypeArray type = { .kind = TYPE_ARRAY, .base = base, .size = size };
    return (TypeArray*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeArray));
}

TypeFunction* createFunction(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments) {
    TypeFunction type = { .kind = TYPE_FUNCTION, .ret_type = ret_type, .arguments = arguments, .arg_count = arg_count };
    return (TypeFunction*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeFunction));
}

void buildTypeNameInto(String* dst, Type* type) {
    ASSERT(type != NULL);
    switch (type->kind) {
        case TYPE_ERROR: {
            break;
        }
        case TYPE_NEVER: {
            *dst = pushToString(*dst, str("!"));
            break;
        }
        case TYPE_VOID: {
            *dst = pushToString(*dst, str("()"));
            break;
        }
        case TYPE_INT: {
            TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
            pushFormattedString(dst, "i%zi", t->size);
            break;
        }
        case TYPE_UINT: {
            TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
            pushFormattedString(dst, "u%zi", t->size);
            break;
        }
        case TYPE_REAL: {
            TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
            pushFormattedString(dst, "f%zi", t->size);
            break;
        }
        case TYPE_POINTER: {
            TypePointer* t = (TypePointer*)type;
            *dst = pushToString(*dst, str("*"));
            buildTypeNameInto(dst, t->base);
            break;
        }
        case TYPE_ARRAY: {
            TypeArray* t = (TypeArray*)type;
            pushFormattedString(dst, "[%zi]", t->size);
            buildTypeNameInto(dst, t->base);
            break;
        }
        case TYPE_FUNCTION: {
            TypeFunction* t = (TypeFunction*)type;
            *dst = pushToString(*dst, str("fn ("));
            for (size_t i = 0; i < t->arg_count; i++) {
                if (i != 0) {
                    *dst = pushToString(*dst, str(", "));
                }
                buildTypeNameInto(dst, t->arguments[i]);
            }
            *dst = pushToString(*dst, str("): "));
            buildTypeNameInto(dst, t->ret_type);
            break;
        }
    }
}

String buildTypeName(Type* type) {
    String ret = createEmptyString();
    buildTypeNameInto(&ret, type);
    return ret;
}

