
#include <stdbool.h>
#include <string.h>

#include "compiler/variable.h"
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

static bool isIndexValid(const TypeContext* table, size_t idx) {
    return table->types[idx] != NULL;
}

static void freeType(Type* type) {
    if (type != NULL) {
        switch (type->kind) {
            case TYPE_ERROR:
            case TYPE_NEVER:
            case TYPE_VOID:
            case TYPE_BOOL:
            case TYPE_INT:
            case TYPE_UINT:
            case TYPE_REAL:
            case TYPE_POINTER:
            case TYPE_ARRAY:
            case TYPE_REFERENCE:
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

static bool shallowCompareTypes(const Type* a, const Type* b) {
    ASSERT(a != NULL);
    ASSERT(b != NULL);
    if (a->kind != b->kind) {
        return false;
    } else {
        switch (a->kind) {
            case TYPE_ERROR:
            case TYPE_NEVER:
            case TYPE_VOID:
            case TYPE_BOOL:
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
            case TYPE_REFERENCE: {
                TypeReference* ta = (TypeReference*)a;
                TypeReference* tb = (TypeReference*)b;
                return ta->binding == tb->binding;
            }
        }
        UNREACHABLE(", unhandled type kind");
    }
}

static size_t shallowHashType(const Type* type) {
    ASSERT(type != NULL);
    switch (type->kind) {
        case TYPE_ERROR:
        case TYPE_NEVER:
        case TYPE_VOID:
        case TYPE_BOOL:
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
        case TYPE_REFERENCE: {
            TypeReference* t = (TypeReference*)type;
            return hashCombine(hashInt(t->kind), hashInt((size_t)t->binding));
        }
    }
    UNREACHABLE(", unhandled type kind");
}

static bool continueSearch(const TypeContext* table, size_t idx, const Type* key) {
    return table->types[idx] != NULL && !shallowCompareTypes(table->types[idx], key);
}

static size_t findIndexHashTable(const TypeContext* table, const Type* key) {
    size_t idx = shallowHashType(key) % table->capacity;
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
    if (table->capacity == 0 || 2 * table->capacity < 3 * table->count) {
        rebuildHashTable(table, (table->capacity == 0 ? INITIAL_CAPACITY : 3 * table->capacity / 2));
    }
}

static Type* createTypeIfAbsent(TypeContext* context, Type* type, size_t size, bool* new) {
    tryResizingHashTable(context);
    size_t idx = findIndexHashTable(context, type);
    if (!isIndexValid(context, idx)) {
        context->types[idx] = checkedAlloc(size);
        memcpy(context->types[idx], type, size);
        context->count++;
        if (new != NULL) {
            *new = true;
        }
        context->types[idx]->equivalent = context->types[idx];
    } else if (new != NULL) {
        *new = false;
    }
    return context->types[idx];
}

Type* createUnsizedPrimitiveType(TypeContext* cxt, TypeKind kind) {
    Type type = { .kind = kind };
    return createTypeIfAbsent(cxt, &type, sizeof(Type), NULL);
}

TypeSizedPrimitive* createSizedPrimitiveType(TypeContext* cxt, TypeKind kind, size_t size) {
    TypeSizedPrimitive type = { .kind = kind, .size = size };
    return (TypeSizedPrimitive*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeSizedPrimitive), NULL);
}

TypePointer* createPointerType(TypeContext* cxt, Type* base) {
    TypePointer type = { .kind = TYPE_POINTER, .base = base };
    return (TypePointer*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypePointer), NULL);
}

TypeArray* createArrayType(TypeContext* cxt, Type* base, size_t size) {
    TypeArray type = { .kind = TYPE_ARRAY, .base = base, .size = size };
    return (TypeArray*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeArray), NULL);
}

TypeFunction* createFunctionType(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments) {
    TypeFunction type = { .kind = TYPE_FUNCTION, .ret_type = ret_type, .arguments = arguments, .arg_count = arg_count };
    bool new;
    TypeFunction* ret = (TypeFunction*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeFunction), &new);
    if (!new) {
        FREE(arguments);
    }
    return ret;
}

TypeReference* createTypeReference(TypeContext* cxt, struct SymbolType* binding) {
    TypeReference type = { .kind = TYPE_REFERENCE, .binding = binding };
    return (TypeReference*)createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeReference), NULL);
}

static void buildTypeNameInto(String* dst, const Type* type) {
    if (type == NULL) {
        *dst = pushToString(*dst, str("_"));
    } else {
        switch (type->kind) {
            case TYPE_ERROR: {
                *dst = pushToString(*dst, str("error"));
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
            case TYPE_BOOL: {
                *dst = pushToString(*dst, str("bool"));
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
            case TYPE_REFERENCE: {
                TypeReference* t = (TypeReference*)type;
                *dst = pushToString(*dst, str(t->binding->name));
                break;
            }
        }
    }
}

String buildTypeName(const Type* type) {
    String ret = createEmptyString();
    buildTypeNameInto(&ret, type);
    return ret;
}

typedef struct TypeReferenceStack {
    struct TypeReferenceStack* last;
    const SymbolType* binding;
} TypeReferenceStack;

#define CYCLIC_CHECK(TYPE, NAME, TRUE, DEFAULT)                                         \
    static TYPE* NAME ## Helper (Type* type, TypeReferenceStack* stack) {               \
        TRUE else if (type->kind == TYPE_REFERENCE) {                                   \
            TypeReference* t = (TypeReference*)type;                                    \
            TypeReferenceStack elem = {                                                 \
                .last = stack,                                                          \
                .binding = t->binding                                                   \
            };                                                                          \
            TypeReferenceStack* cur = stack;                                            \
            while (cur != NULL) {                                                       \
                if (cur->binding != elem.binding) {                                     \
                    cur = cur->last;                                                    \
                } DEFAULT                                                               \
            }                                                                           \
            return NAME ## Helper (t->binding->type, &elem);                            \
        } DEFAULT                                                                       \
    }                                                                                   \
    TYPE* NAME (Type* type) {                                                           \
        return NAME ## Helper (type, NULL);                                             \
    }

CYCLIC_CHECK(
    TypeSizedPrimitive, isSignedIntegerType,
    if (type->kind == TYPE_INT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isUnsignedIntegerType,
    if (type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isIntegerType,
    if (type->kind == TYPE_INT || type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isFloatType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 32) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isDoubleType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 64) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isRealType,
    if (type->kind == TYPE_REAL) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeSizedPrimitive, isNumericType,
    if (type->kind == TYPE_REAL || type->kind == TYPE_INT || type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    Type, isBooleanType,
    if (type->kind == TYPE_BOOL) { return type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypePointer, isPointerType,
    if (type->kind == TYPE_POINTER) { return (TypePointer*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeArray, isArrayType,
    if (type->kind == TYPE_ARRAY) { return (TypeArray*)type; },
    else { return NULL; }
)

CYCLIC_CHECK(
    TypeFunction, isFunctionType,
    if (type->kind == TYPE_FUNCTION) { return (TypeFunction*)type; },
    else { return NULL; }
)

typedef struct DoubleTypeReferenceStack {
    struct DoubleTypeReferenceStack* last;
    Type* types[2];
} DoubleTypeReferenceStack;

static bool compareStructuralTypesHelper(Type* a, Type* b, DoubleTypeReferenceStack* stack) {
    if (a == b) {
        return true;
    } else if (a == NULL || b == NULL) {
        return false;
    } else if (a->kind != b->kind) {
        if (a->kind == TYPE_REFERENCE) {
            TypeReference* t = (TypeReference*)a;
            if (t->binding->type == b) {
                return true;
            } else {
                DoubleTypeReferenceStack elem = {
                    .last = stack, .types = { a, b }
                };
                DoubleTypeReferenceStack* cur = stack;
                while (cur != NULL) {
                    if (cur->types[0] != a || cur->types[1] != b) {
                        cur = cur->last;
                    } else {
                        return true;
                    }
                }
                return compareStructuralTypesHelper(t->binding->type, b, &elem);
            }
        } else if (b->kind == TYPE_REFERENCE) {
            TypeReference* t = (TypeReference*)b;
            if (a == t->binding->type) {
                return true;
            } else {
                DoubleTypeReferenceStack elem = {
                    .last = stack, .types = { a, b }
                };
                DoubleTypeReferenceStack* cur = stack;
                while (cur != NULL) {
                    if (cur->types[0] != a || cur->types[1] != b) {
                        cur = cur->last;
                    } else {
                        return true;
                    }
                }
                return compareStructuralTypesHelper(a, t->binding->type, &elem);
            }
        } else {
            return false;
        }
    } else {
        switch (a->kind) {
            case TYPE_ERROR:
            case TYPE_NEVER:
            case TYPE_VOID:
            case TYPE_BOOL:
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
                return compareStructuralTypesHelper(ta->base, tb->base, stack);
            }
            case TYPE_ARRAY: {
                TypeArray* ta = (TypeArray*)a;
                TypeArray* tb = (TypeArray*)b;
                return ta->size == tb->size && compareStructuralTypesHelper(ta->base, tb->base, stack);
            }
            case TYPE_FUNCTION: {
                TypeFunction* ta = (TypeFunction*)a;
                TypeFunction* tb = (TypeFunction*)b;
                if (ta->arg_count != tb->arg_count || compareStructuralTypesHelper(ta->ret_type, tb->ret_type, stack)) {
                    return false;
                } else {
                    for (size_t i = 0; i < ta->arg_count; i++) {
                        if (!compareStructuralTypesHelper(ta->arguments[i], tb->arguments[i], stack)) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            case TYPE_REFERENCE: {
                TypeReference* ta = (TypeReference*)a;
                TypeReference* tb = (TypeReference*)b;
                if (ta->binding->type == tb->binding->type) {
                    return true;
                } else {
                    DoubleTypeReferenceStack elem = {
                        .last = stack, .types = { a, b }
                    };
                    DoubleTypeReferenceStack* cur = stack;
                    while (cur != NULL) {
                        if (cur->types[0] != a || cur->types[1] != b) {
                            cur = cur->last;
                        } else {
                            return true;
                        }
                    }
                    return compareStructuralTypesHelper(ta->binding->type, tb->binding->type, &elem);
                }
            }
        }
        UNREACHABLE(", unhandled type kind");
    }
}

bool isErrorType(Type* type) {
    return type->kind == TYPE_ERROR;
}

bool compareStructuralTypes(Type* a, Type* b) {
    return compareStructuralTypesHelper(a, b, NULL);
}

