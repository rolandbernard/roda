
#include <stdbool.h>
#include <string.h>

#include "compiler/variable.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "util/alloc.h"
#include "util/hash.h"
#include "util/sort.h"

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
            case TYPE_VOID:
            case TYPE_BOOL:
            case TYPE_INT:
            case TYPE_UINT:
            case TYPE_REAL:
            case TYPE_POINTER:
            case TYPE_ARRAY:
            case TYPE_REFERENCE:
            case TYPE_UNSURE:
                break;
            case TYPE_FUNCTION: {
                TypeFunction* t = (TypeFunction*)type;
                FREE(t->arguments);
                break;
            }
            case TYPE_STRUCT: {
                TypeStruct* t = (TypeStruct*)type;
                FREE(t->names);
                FREE(t->types);
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
                if (ta->arg_count != tb->arg_count || ta->ret_type != tb->ret_type || ta->vararg != tb->vararg) {
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
            case TYPE_STRUCT: {
                TypeStruct* ta = (TypeStruct*)a;
                TypeStruct* tb = (TypeStruct*)b;
                if (ta->count != tb->count) {
                    return false;
                } else {
                    for (size_t i = 0; i < ta->count; i++) {
                        if (ta->names[i] != tb->names[i] || ta->types[i] != tb->types[i]) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            case TYPE_UNSURE: {
                return a == b; // Note: unsure types are not uniqued
            }
        }
        UNREACHABLE("unhandled type kind");
    }
}

static size_t shallowHashType(const Type* type) {
    ASSERT(type != NULL);
    switch (type->kind) {
        case TYPE_ERROR:
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
            size_t hash = hashCombine(
                hashCombine(hashInt(t->kind), hashInt((size_t)t->ret_type)),
                hashCombine(hashInt(t->arg_count), hashInt(t->vararg))
            );
            for (size_t i = 0; i < t->arg_count; i++) {
                hash = hashCombine(hash, hashInt((size_t)t->arguments[i]));
            }
            return hash;
        }
        case TYPE_REFERENCE: {
            TypeReference* t = (TypeReference*)type;
            return hashCombine(hashInt(t->kind), hashInt((size_t)t->binding));
        }
        case TYPE_STRUCT: {
            TypeStruct* t = (TypeStruct*)type;
            size_t hash = hashCombine(hashInt(t->kind), hashInt((size_t)t->count));
            for (size_t i = 0; i < t->count; i++) {
                hash = hashCombine(hash, hashInt((size_t)t->names[i]));
                hash = hashCombine(hash, hashInt((size_t)t->types[i]));
            }
            return hash;
        }
        case TYPE_UNSURE: {
            return hashInt((size_t)type); // Note: unsure types are not uniqued
        }
    }
    UNREACHABLE("unhandled type kind");
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
        context->types[idx]->codegen = NULL;
    } else if (new != NULL) {
        *new = false;
    }
    return context->types[idx];
}

Type* createUnsizedPrimitiveType(TypeContext* cxt, TypeKind kind) {
    Type type = { .kind = kind };
    return createTypeIfAbsent(cxt, &type, sizeof(Type), NULL);
}

Type* createSizedPrimitiveType(TypeContext* cxt, TypeKind kind, size_t size) {
    TypeSizedPrimitive type = { .kind = kind, .size = size };
    return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeSizedPrimitive), NULL);
}

Type* createPointerType(TypeContext* cxt, Type* base) {
    if (isErrorType(base)) {
        return base;
    } else {
        TypePointer type = { .kind = TYPE_POINTER, .base = base };
        return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypePointer), NULL);
    }
}

Type* createArrayType(TypeContext* cxt, Type* base, size_t size) {
    if (isErrorType(base)) {
        return base;
    } else {
        TypeArray type = { .kind = TYPE_ARRAY, .base = base, .size = size };
        return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeArray), NULL);
    }
}

Type* createFunctionType(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments, bool vararg) {
    if (isErrorType(ret_type)) {
        FREE(arguments);
        return ret_type;
    } else {
        for (size_t i = 0; i < arg_count; i++) {
            if (isErrorType(arguments[i])) {
                Type* ret = arguments[i];
                FREE(arguments);
                return ret;
            }
        }
        TypeFunction type = {
            .kind = TYPE_FUNCTION,
            .ret_type = ret_type,
            .arguments = arguments,
            .arg_count = arg_count,
            .vararg = vararg,
        };
        bool new;
        Type* ret = createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeFunction), &new);
        if (!new) {
            FREE(arguments);
        }
        return ret;
    }
}

Type* createTypeReference(TypeContext* cxt, struct SymbolType* binding) {
    TypeReference type = { .kind = TYPE_REFERENCE, .binding = binding };
    return createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeReference), NULL);
}

Type* createTypeStruct(TypeContext* cxt, Symbol* names, Type** types, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (isErrorType(types[i])) {
            Type* ret = types[i];
            FREE(names);
            FREE(types);
            return ret;
        }
    }
    TypeStruct type = {
        .kind = TYPE_STRUCT, .names = names, .types = types, .count = count,
    };
    bool new;
    Type* ret = createTypeIfAbsent(cxt, (Type*)&type, sizeof(TypeStruct), &new);
    if (!new) {
        FREE(names);
        FREE(types);
    }
    return ret;
}

Type* createUnsureType(TypeContext* cxt, Type* fallback) {
    TypeUnsure* type = NEW(TypeUnsure);
    type->kind = TYPE_UNSURE;
    type->codegen = NULL;
    type->fallback = fallback;
    type->actual = NULL;
    tryResizingHashTable(cxt);
    size_t idx = findIndexHashTable(cxt, (Type*)type);
    ASSERT(!isIndexValid(cxt, idx));
    cxt->types[idx] = (Type*)type;
    cxt->count++;
    return (Type*)type;
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
                if (t->vararg) {
                    *dst = pushToString(*dst, str(", .."));
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
            case TYPE_STRUCT: {
                TypeStruct* t = (TypeStruct*)type;
                *dst = pushToString(*dst, str("("));
                for (size_t i = 0; i < t->count; i++) {
                    if (i != 0) {
                        *dst = pushToString(*dst, str(", "));
                    }
                    *dst = pushToString(*dst, str(t->names[i]));
                    *dst = pushToString(*dst, str(" = "));
                    buildTypeNameInto(dst, t->types[i]);
                }
                *dst = pushToString(*dst, str(")"));
                break;
            }
            case TYPE_UNSURE: {
                TypeUnsure* t = (TypeUnsure*)type;
                if (t->actual != NULL) {
                    buildTypeNameInto(dst, t->actual);
                } else {
                    buildTypeNameInto(dst, t->fallback);
                }
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

#define STRUCTURAL_TYPE_CHECK(TYPE, NAME, TRUE, DEFAULT)                        \
    static TYPE NAME ## Helper (Type* type, TypeReferenceStack* stack) {        \
        TRUE else if (type->kind == TYPE_REFERENCE) {                           \
            TypeReference* t = (TypeReference*)type;                            \
            TypeReferenceStack elem = {                                         \
                .last = stack,                                                  \
                .binding = t->binding                                           \
            };                                                                  \
            TypeReferenceStack* cur = stack;                                    \
            while (cur != NULL) {                                               \
                if (cur->binding != elem.binding) {                             \
                    cur = cur->last;                                            \
                } DEFAULT                                                       \
            }                                                                   \
            return NAME ## Helper (t->binding->type, &elem);                    \
        } else if (type->kind == TYPE_UNSURE) {                                 \
            TypeUnsure* t = (TypeUnsure*)type;                                  \
            if (t->actual != NULL) {                                            \
                return NAME ## Helper (t->actual, stack);                       \
            } else {                                                            \
                return NAME ## Helper (t->fallback, stack);                     \
            }                                                                   \
        } DEFAULT                                                               \
    }                                                                           \
    TYPE NAME (Type* type) {                                                    \
        return NAME ## Helper (type, NULL);                                     \
    }

static Type* isTypeOfKindHelper(Type* type, TypeKind kind, TypeReferenceStack* stack) {
    if (type->kind == kind) {
        return type;
    } else if (type->kind == TYPE_REFERENCE) {
        TypeReference* t = (TypeReference*)type;
        TypeReferenceStack elem = { .last = stack, .binding = t->binding };
        TypeReferenceStack* cur = stack;
        while (cur != NULL) {
            if (cur->binding != elem.binding) {
                cur = cur->last;
            } else {
                return NULL;
            }
        }
        return isTypeOfKindHelper(t->binding->type, kind, &elem);
    } else if (type->kind == TYPE_UNSURE) {
        TypeUnsure* t = (TypeUnsure*)type;
        if (t->actual != NULL) {
            return isTypeOfKindHelper(t->actual, kind, stack);
        } else {
            return isTypeOfKindHelper(t->fallback, kind, stack);
        }
    } else {
        return NULL;
    }
}

static Type* isTypeOfKind(Type* type, TypeKind kind) {
    return isTypeOfKindHelper(type, kind, NULL);
}

TypeSizedPrimitive* isSignedIntegerType(Type* type) {
    return (TypeSizedPrimitive*)isTypeOfKind(type, TYPE_INT);
}

TypeSizedPrimitive* isUnsignedIntegerType(Type* type) {
    return (TypeSizedPrimitive*)isTypeOfKind(type, TYPE_UINT);
}

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isIntegerType,
    if (type->kind == TYPE_INT || type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isFloatType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 32) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isDoubleType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 64) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

TypeSizedPrimitive* isRealType(Type* type) {
    return (TypeSizedPrimitive*)isTypeOfKind(type, TYPE_REAL);
}

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isNumericType,
    if (type->kind == TYPE_REAL || type->kind == TYPE_INT || type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; }
)

Type* isBooleanType(Type* type) {
    return isTypeOfKind(type, TYPE_BOOL);
}

Type* isVoidType(Type* type) {
    return isTypeOfKind(type, TYPE_VOID);
}

TypePointer* isPointerType(Type* type) {
    return (TypePointer*)isTypeOfKind(type, TYPE_POINTER);
}

TypeArray* isArrayType(Type* type) {
    return (TypeArray*)isTypeOfKind(type, TYPE_ARRAY);
}

TypeFunction* isFunctionType(Type* type) {
    return (TypeFunction*)isTypeOfKind(type, TYPE_FUNCTION);
}

TypeStruct* isStructType(Type* type) {
    return (TypeStruct*)isTypeOfKind(type, TYPE_STRUCT);
}

size_t lookupIndexOfStructField(TypeStruct* type, Symbol name) {
    for (size_t i = 0; i < type->count; i++) {
        if (type->names[i] == name) {
            return i;
        }
    }
    return NO_POS;
}

bool isErrorType(Type* type) {
    return type != NULL && type->kind == TYPE_ERROR;
}

STRUCTURAL_TYPE_CHECK(
    bool, isValidType,
    if (
        type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL || type->kind == TYPE_INT
        || type->kind == TYPE_UINT || type->kind == TYPE_REAL || type->kind == TYPE_POINTER || type->kind == TYPE_FUNCTION
    ) {
        return true;
    } else if (type->kind == TYPE_ARRAY) {
        TypeArray* array = (TypeArray*)type;
        return isValidTypeHelper(array->base, stack);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isValidTypeHelper(s->types[i], stack)) {
                return false;
            }
        }
        return true;
    },
    else { return false; }
)

STRUCTURAL_TYPE_CHECK(
    bool, isEffectivelyVoidType,
    if (type->kind == TYPE_VOID) {
        return true;
    } else if (
        type->kind == TYPE_ERROR || type->kind == TYPE_BOOL || type->kind == TYPE_INT || type->kind == TYPE_UINT
        || type->kind == TYPE_REAL || type->kind == TYPE_POINTER || type->kind == TYPE_FUNCTION
    ) {
        return false;
    } else if (type->kind == TYPE_ARRAY) {
        TypeArray* array = (TypeArray*)type;
        return array->size == 0 || isEffectivelyVoidTypeHelper(array->base, stack);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isEffectivelyVoidTypeHelper(s->types[i], stack)) {
                return false;
            }
        }
        return true;
    },
    else { return true; }
)

STRUCTURAL_TYPE_CHECK(
    bool, isSizedType,
    if (type->kind == TYPE_VOID) {
        return true;
    } else if (type->kind == TYPE_FUNCTION) {
        return false;
    } else if (
        type->kind == TYPE_ERROR || type->kind == TYPE_BOOL || type->kind == TYPE_INT
        || type->kind == TYPE_UINT || type->kind == TYPE_REAL || type->kind == TYPE_POINTER
    ) {
        return true;
    } else if (type->kind == TYPE_ARRAY) {
        TypeArray* array = (TypeArray*)type;
        return isSizedTypeHelper(array->base, stack);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isSizedTypeHelper(s->types[i], stack)) {
                return false;
            }
        }
        return true;
    },
    else { return false; }
)

typedef struct DoubleTypeReferenceStack {
    struct DoubleTypeReferenceStack* last;
    Type* types[2];
} DoubleTypeReferenceStack;

static bool compareStructuralTypesHelper(Type* a, Type* b, bool change, DoubleTypeReferenceStack* stack) {
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
                return compareStructuralTypesHelper(t->binding->type, b, change, &elem);
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
                return compareStructuralTypesHelper(a, t->binding->type, change, &elem);
            }
        } else if (a->kind == TYPE_UNSURE) {
            TypeUnsure* t = (TypeUnsure*)a;
            if (t->actual != NULL) {
                return compareStructuralTypesHelper(t->actual, b, change, stack);
            } else if (change) {
                t->actual = b;
                return true;
            } else {
                return compareStructuralTypesHelper(t->fallback, b, change, stack);
            }
        } else if (b->kind == TYPE_UNSURE) {
            TypeUnsure* t = (TypeUnsure*)b;
            if (t->actual != NULL) {
                return compareStructuralTypesHelper(a, t->actual, change, stack);
            } else if (change) {
                t->actual = a;
                return true;
            } else {
                return compareStructuralTypesHelper(a, t->fallback, change, stack);
            }
        } else {
            return false;
        }
    } else {
        switch (a->kind) {
            case TYPE_UNSURE: {
                TypeUnsure* ta = (TypeUnsure*)a;
                TypeUnsure* tb = (TypeUnsure*)b;
                if (ta->actual == NULL) {
                    if (change) {
                        ta->actual = b;
                        return true;
                    } else if (tb->actual != NULL) {
                        return compareStructuralTypesHelper(ta->fallback, tb->actual, change, stack);
                    } else {
                        return compareStructuralTypesHelper(ta->fallback, tb->fallback, change, stack);
                    }
                } else if (tb->actual == NULL) {
                    if (change) {
                        tb->actual = a;
                        return true;
                    } else {
                        return compareStructuralTypesHelper(ta->actual, tb->fallback, change, stack);
                    }
                } else {
                    return compareStructuralTypesHelper(ta->actual, tb->actual, change, stack);
                }
            }
            case TYPE_ERROR:
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
                return compareStructuralTypesHelper(ta->base, tb->base, change, stack);
            }
            case TYPE_ARRAY: {
                TypeArray* ta = (TypeArray*)a;
                TypeArray* tb = (TypeArray*)b;
                return ta->size == tb->size && compareStructuralTypesHelper(ta->base, tb->base, change, stack);
            }
            case TYPE_FUNCTION: {
                TypeFunction* ta = (TypeFunction*)a;
                TypeFunction* tb = (TypeFunction*)b;
                if (ta->arg_count != tb->arg_count || ta->vararg != tb->vararg || compareStructuralTypesHelper(ta->ret_type, tb->ret_type, change, stack)) {
                    return false;
                } else {
                    for (size_t i = 0; i < ta->arg_count; i++) {
                        if (!compareStructuralTypesHelper(ta->arguments[i], tb->arguments[i], change, stack)) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            case TYPE_STRUCT: {
                TypeStruct* ta = (TypeStruct*)a;
                TypeStruct* tb = (TypeStruct*)b;
                if (ta->count != tb->count) {
                    return false;
                } else {
                    for (size_t i = 0; i < ta->count; i++) {
                        if (ta->names[i] != tb->names[i]) {
                            return false;
                        }
                    }
                    for (size_t i = 0; i < ta->count; i++) {
                        if (!compareStructuralTypesHelper(ta->types[i], tb->types[i], change, stack)) {
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
                    return compareStructuralTypesHelper(ta->binding->type, tb->binding->type, change, &elem);
                }
            }
        }
        UNREACHABLE("unhandled type kind");
    }
}

bool compareStructuralTypes(Type* a, Type* b) {
    return compareStructuralTypesHelper(a, b, false, NULL);
}

bool assertStructuralTypesEquality(Type* a, Type* b) {
    return compareStructuralTypesHelper(a, b, true, NULL);
}

