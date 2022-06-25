
#include <stdbool.h>
#include <string.h>

#include "compiler/variable.h"
#include "compiler/typeeval.h"
#include "errors/fatalerror.h"
#include "text/format.h"
#include "util/alloc.h"
#include "util/hash.h"
#include "util/sort.h"

#include "compiler/types.h"

#define INITIAL_CAPACITY 32

void initTypeContext(TypeContext* cxt) {
    cxt->types = NULL;
    cxt->error = createUnsizedPrimitiveType(cxt, NULL, TYPE_ERROR);
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
    Type* cur = cxt->types;
    while (cur != NULL) {
        Type* next = cur->next;
        freeType(cur);
        cur = next;
    }
}

Type* getErrorType(TypeContext* cxt) {
    return cxt->error;
}

static Type* addAndInitType(TypeContext* context, Type* type, TypeKind kind, AstNode* def) {
    type->kind = kind;
    type->codegen = NULL;
    type->equiv = type;
    type->def = def;
    type->next = context->types;
    context->types = type;
    return type;
}

Type* createUnsizedPrimitiveType(TypeContext* cxt, struct AstNode* def, TypeKind kind) {
    Type* type = NEW(Type);
    return addAndInitType(cxt, type, kind, def);
}

Type* createSizedPrimitiveType(TypeContext* cxt, struct AstNode* def, TypeKind kind, size_t size) {
    TypeSizedPrimitive* type = NEW(TypeSizedPrimitive);
    type->size = size;
    return addAndInitType(cxt, (Type*)type, kind, def);
}

Type* createPointerType(TypeContext* cxt, struct AstNode* def, Type* base) {
    if (isErrorType(base)) {
        return base;
    } else {
        TypePointer* type = NEW(TypePointer);
        type->base = base;
        return addAndInitType(cxt, (Type*)type, TYPE_POINTER, def);
    }
}

Type* createArrayType(TypeContext* cxt, struct AstNode* def, Type* base, size_t size) {
    if (isErrorType(base)) {
        return base;
    } else {
        TypeArray* type = NEW(TypeArray);
        type->base = base;
        type->size = size;
        return addAndInitType(cxt, (Type*)type, TYPE_ARRAY, def);
    }
}

Type* createFunctionType(TypeContext* cxt, struct AstNode* def, Type* ret_type, size_t arg_count, Type** arguments, bool vararg) {
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
        TypeFunction* type = NEW(TypeFunction);
        type->ret_type = ret_type;
        type->arguments = arguments;
        type->arg_count = arg_count;
        type->vararg = vararg;
        return addAndInitType(cxt, (Type*)type, TYPE_FUNCTION, def);
    }
}

Type* createTypeReference(TypeContext* cxt, struct AstNode* def, struct SymbolType* binding) {
    TypeReference* type = NEW(TypeReference);
    type->binding = binding;
    return addAndInitType(cxt, (Type*)type, TYPE_REFERENCE, def);
}

Type* createTypeStruct(TypeContext* cxt, struct AstNode* def, Symbol* names, Type** types, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (isErrorType(types[i])) {
            Type* ret = types[i];
            FREE(names);
            FREE(types);
            return ret;
        }
    }
    TypeStruct* type = NEW(TypeStruct);
    type->names = names;
    type->types = types;
    type->count = count;
    return addAndInitType(cxt, (Type*)type, TYPE_STRUCT, def);
}

Type* createUnsureType(TypeContext* cxt, struct AstNode* def, Type* fallback) {
    TypeUnsure* type = NEW(TypeUnsure);
    type->fallback = fallback;
    type->actual = NULL;
    return addAndInitType(cxt, (Type*)type, TYPE_UNSURE, def);
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
                if (t->size != SIZE_SIZE) {
                    pushFormattedString(dst, "i%zi", t->size);
                } else {
                    pushFormattedString(dst, "isize", t->size);
                }
                break;
            }
            case TYPE_UINT: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                if (t->size != SIZE_SIZE) {
                    pushFormattedString(dst, "u%zi", t->size);
                } else {
                    pushFormattedString(dst, "usize", t->size);
                }
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

#define STRUCTURAL_TYPE_CHECK_HELPER(NAME, TRUE, DEFAULT, ON_NULL, CXT)     \
    if (type != NULL) {                                                     \
        TRUE else if (type->kind == TYPE_REFERENCE) {                       \
            TypeReference* t = (TypeReference*)type;                        \
            TypeReferenceStack elem = {                                     \
                .last = stack,                                              \
                .binding = t->binding                                       \
            };                                                              \
            TypeReferenceStack* cur = stack;                                \
            while (cur != NULL) {                                           \
                if (cur->binding != elem.binding) {                         \
                    cur = cur->last;                                        \
                } DEFAULT                                                   \
            }                                                               \
            return NAME (t->binding->type, &elem, CXT);                     \
        } else if (type->kind == TYPE_UNSURE) {                             \
            TypeUnsure* t = (TypeUnsure*)type;                              \
            if (t->actual != NULL) {                                        \
                return NAME (t->actual, stack, CXT);                        \
            } else {                                                        \
                return NAME (t->fallback, stack, CXT);                      \
            }                                                               \
    } DEFAULT                                                               \
    } ON_NULL

#define STRUCTURAL_TYPE_CHECK(TYPE, NAME, TRUE, DEFAULT, ON_NULL)                       \
    static TYPE NAME ## Helper (Type* type, TypeReferenceStack* stack, void* _c) {      \
        STRUCTURAL_TYPE_CHECK_HELPER(NAME ## Helper, TRUE, DEFAULT, ON_NULL, _c)        \
    }                                                                                   \
    TYPE NAME (Type* type) {                                                            \
        return NAME ## Helper (type, NULL, NULL);                                       \
    }

static Type* isTypeOfKindHelper(Type* type, TypeReferenceStack* stack, TypeKind kind) {
    STRUCTURAL_TYPE_CHECK_HELPER(isTypeOfKindHelper, 
        if (type->kind == kind) { return type; },
        else { return NULL; },
        else { return NULL; },
        kind
    )
}

static Type* isTypeOfKind(Type* type, TypeKind kind) {
    return isTypeOfKindHelper(type, NULL, kind);
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
    else { return NULL; },
    else { return NULL; }
)

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isFloatType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 32) { return (TypeSizedPrimitive*)type; },
    else { return NULL; },
    else { return NULL; }
)

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isDoubleType,
    if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 64) { return (TypeSizedPrimitive*)type; },
    else { return NULL; },
    else { return NULL; }
)

TypeSizedPrimitive* isRealType(Type* type) {
    return (TypeSizedPrimitive*)isTypeOfKind(type, TYPE_REAL);
}

STRUCTURAL_TYPE_CHECK(
    TypeSizedPrimitive*, isNumericType,
    if (type->kind == TYPE_REAL || type->kind == TYPE_INT || type->kind == TYPE_UINT) { return (TypeSizedPrimitive*)type; },
    else { return NULL; },
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

TypeReference* isTypeReference(Type* type) {
    return (TypeReference*)isTypeOfKind(type, TYPE_REFERENCE);
}

TypeUnsure* isUnsureType(Type* type) {
    return (TypeUnsure*)isTypeOfKind(type, TYPE_UNSURE);
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
    bool, isPartialType,
    if (
        type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL
        || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL
    ) {
        return false;
    } else if (type->kind == TYPE_POINTER) {
        TypePointer* t = (TypePointer*)type;
        return isPartialTypeHelper(t->base, stack, NULL);
    } else if (type->kind == TYPE_FUNCTION) {
        TypeFunction* t = (TypeFunction*)type;
        for (size_t i = 0; i < t->arg_count; i++) {
            if (isPartialTypeHelper(t->arguments[i], stack, NULL)) {
                return true;
            }
        }
        return isPartialTypeHelper(t->ret_type, stack, NULL);
    } else if (type->kind == TYPE_ARRAY) {
        TypeArray* array = (TypeArray*)type;
        return isPartialTypeHelper(array->base, stack, NULL);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (isPartialTypeHelper(s->types[i], stack, NULL)) {
                return true;
            }
        }
        return false;
    },
    else { return false; },
    else { return true; }
)

STRUCTURAL_TYPE_CHECK(
    bool, containsErrorType,
    if (type->kind == TYPE_ERROR) {
        return true;
    } else if (type->kind == TYPE_VOID || type->kind == TYPE_BOOL || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL) {
        return false;
    } else if (type->kind == TYPE_POINTER) {
        TypePointer* t = (TypePointer*)type;
        return containsErrorTypeHelper(t->base, stack, NULL);
    } else if (type->kind == TYPE_FUNCTION) {
        TypeFunction* t = (TypeFunction*)type;
        for (size_t i = 0; i < t->arg_count; i++) {
            if (containsErrorTypeHelper(t->arguments[i], stack, NULL)) {
                return true;
            }
        }
        return containsErrorTypeHelper(t->ret_type, stack, NULL);
    } else if (type->kind == TYPE_ARRAY) {
        TypeArray* array = (TypeArray*)type;
        return containsErrorTypeHelper(array->base, stack, NULL);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (containsErrorTypeHelper(s->types[i], stack, NULL)) {
                return true;
            }
        }
        return false;
    },
    else { return false; },
    else { return false; }
)

static void* fillPartialTypeHelper(Type* type, TypeReferenceStack* stack, Type* with) {
    STRUCTURAL_TYPE_CHECK_HELPER(fillPartialTypeHelper, 
        if (
            type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL
            || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL
        ) {
            return NULL;
        } else if (type->kind == TYPE_POINTER) {
            TypePointer* t = (TypePointer*)type;
            fillPartialTypeHelper(t->base, stack, with);
            return NULL;
        } else if (type->kind == TYPE_FUNCTION) {
            TypeFunction* t = (TypeFunction*)type;
            for (size_t i = 0; i < t->arg_count; i++) {
                fillPartialTypeHelper(t->arguments[i], stack, with);
            }
            fillPartialTypeHelper(t->ret_type, stack, with);
            return NULL;
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            fillPartialTypeHelper(array->base, stack, with);
            return NULL;
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                fillPartialTypeHelper(s->types[i], stack, with);
            }
            return NULL;
        } else if (type->kind == TYPE_UNSURE) {
            TypeUnsure* t = (TypeUnsure*)type;
            if (t->actual != NULL) {
                fillPartialTypeHelper(t->actual, stack, with);
            } else if (t->fallback != NULL) {
                fillPartialTypeHelper(t->fallback, stack, with);
            } else {
                t->fallback = with;
            }
            return NULL;
        },
        else { return NULL; },
        else { return NULL; },
        with
    )
}

void fillPartialType(Type* type, Type* with) {
    fillPartialTypeHelper(type, NULL, with);
}

AstNode* getTypeReason(Type* type) {
    if (type->kind == TYPE_UNSURE) {
        TypeUnsure* t = (TypeUnsure*)type;
        if (t->actual != NULL) {
            return getTypeReason(t->actual);
        } else if (t->fallback != NULL) {
            return getTypeReason(t->fallback);
        } else {
            return NULL;
        }
    } else {
        return type->def;
    }
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
        return isValidTypeHelper(array->base, stack, NULL);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isValidTypeHelper(s->types[i], stack, NULL)) {
                return false;
            }
        }
        return true;
    },
    else { return false; },
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
        return array->size == 0 || isEffectivelyVoidTypeHelper(array->base, stack, NULL);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isEffectivelyVoidTypeHelper(s->types[i], stack, NULL)) {
                return false;
            }
        }
        return true;
    },
    else { return true; },
    else { return false; }
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
        return isSizedTypeHelper(array->base, stack, NULL);
    } else if (type->kind == TYPE_STRUCT) {
        TypeStruct* s = (TypeStruct*)type;
        for (size_t i = 0; i < s->count; i++) {
            if (!isSizedTypeHelper(s->types[i], stack, NULL)) {
                return false;
            }
        }
        return true;
    },
    else { return false; },
    else { return false; }
)

static Type* getEquivType(Type* type) {
    if (type->equiv != type) {
        type->equiv = getEquivType(type->equiv);
    }
    return type->equiv;
}

typedef struct DoubleTypeReferenceStack {
    struct DoubleTypeReferenceStack* last;
    Type* types[2];
} DoubleTypeReferenceStack;

static bool compareStructuralTypesHelper(Type* a, Type* b, bool change, bool* changed, DoubleTypeReferenceStack* stack) {
    if (a == b) {
        return true;
    } else if (a == NULL || b == NULL) {
        return false;
    } else if (getEquivType(a) == getEquivType(b)) {
        return true;
    } else if (a->kind == TYPE_ERROR || b->kind == TYPE_ERROR) {
        return true; // Note: errors could be anything. An error has already been generated.
    } else {
        bool equal = false;
        if (a->kind != b->kind) {
            if (a->kind == TYPE_REFERENCE) {
                TypeReference* t = (TypeReference*)a;
                if (t->binding->type == b) {
                    equal = true;
                } else {
                    DoubleTypeReferenceStack elem = {
                        .last = stack, .types = { a, b }
                    };
                    DoubleTypeReferenceStack* cur = stack;
                    while (cur != NULL) {
                        if (cur->types[0] != a || cur->types[1] != b) {
                            cur = cur->last;
                        } else {
                            equal = true;
                            break;
                        }
                    }
                    if (!equal) {
                        equal = compareStructuralTypesHelper(t->binding->type, b, change, changed, &elem);
                    }
                }
            } else if (b->kind == TYPE_REFERENCE) {
                TypeReference* t = (TypeReference*)b;
                if (a == t->binding->type) {
                    equal = true;
                } else {
                    DoubleTypeReferenceStack elem = {
                        .last = stack, .types = { a, b }
                    };
                    DoubleTypeReferenceStack* cur = stack;
                    while (cur != NULL) {
                        if (cur->types[0] != a || cur->types[1] != b) {
                            cur = cur->last;
                        } else {
                            equal = true;
                            break;
                        }
                    }
                    if (!equal) {
                        equal = compareStructuralTypesHelper(a, t->binding->type, change, changed, &elem);
                    }
                }
            } else if (a->kind == TYPE_UNSURE) {
                TypeUnsure* t = (TypeUnsure*)a;
                if (t->actual != NULL) {
                    equal = compareStructuralTypesHelper(t->actual, b, change, changed, stack);
                } else if (change) {
                    t->actual = b;
                    equal = true;
                    *changed = true;
                } else {
                    equal = compareStructuralTypesHelper(t->fallback, b, change, changed, stack);
                }
            } else if (b->kind == TYPE_UNSURE) {
                TypeUnsure* t = (TypeUnsure*)b;
                if (t->actual != NULL) {
                    equal = compareStructuralTypesHelper(a, t->actual, change, changed, stack);
                } else if (change) {
                    t->actual = a;
                    equal = true;
                    *changed = true;
                } else {
                    equal = compareStructuralTypesHelper(a, t->fallback, change, changed, stack);
                }
            } else {
                equal = false;
            }
        } else {
            switch (a->kind) {
                case TYPE_UNSURE: {
                    TypeUnsure* ta = (TypeUnsure*)a;
                    TypeUnsure* tb = (TypeUnsure*)b;
                    if (ta->actual == NULL) {
                        if (change) {
                            ta->actual = b;
                            equal = true;
                            *changed = true;
                        } else if (tb->actual != NULL) {
                            equal = compareStructuralTypesHelper(ta->fallback, tb->actual, change, changed, stack);
                        } else {
                            equal = compareStructuralTypesHelper(ta->fallback, tb->fallback, change, changed, stack);
                        }
                    } else if (tb->actual == NULL) {
                        if (change) {
                            tb->actual = a;
                            equal = true;
                            *changed = true;
                        } else {
                            equal = compareStructuralTypesHelper(ta->actual, tb->fallback, change, changed, stack);
                        }
                    } else {
                        equal = compareStructuralTypesHelper(ta->actual, tb->actual, change, changed, stack);
                    }
                    break;
                }
                case TYPE_ERROR:
                case TYPE_VOID:
                case TYPE_BOOL:
                    equal = true;
                    break;
                case TYPE_INT:
                case TYPE_UINT:
                case TYPE_REAL: {
                    TypeSizedPrimitive* ta = (TypeSizedPrimitive*)a;
                    TypeSizedPrimitive* tb = (TypeSizedPrimitive*)b;
                    equal = ta->size == tb->size;
                    break;
                }
                case TYPE_POINTER: {
                    TypePointer* ta = (TypePointer*)a;
                    TypePointer* tb = (TypePointer*)b;
                    equal = compareStructuralTypesHelper(ta->base, tb->base, change, changed, stack);
                    break;
                }
                case TYPE_ARRAY: {
                    TypeArray* ta = (TypeArray*)a;
                    TypeArray* tb = (TypeArray*)b;
                    equal = ta->size == tb->size && compareStructuralTypesHelper(ta->base, tb->base, change, changed, stack);
                    break;
                }
                case TYPE_FUNCTION: {
                    TypeFunction* ta = (TypeFunction*)a;
                    TypeFunction* tb = (TypeFunction*)b;
                    if (
                        ta->arg_count != tb->arg_count || ta->vararg != tb->vararg
                        || !compareStructuralTypesHelper(ta->ret_type, tb->ret_type, change, changed, stack)
                    ) {
                        equal = false;
                    } else {
                        equal = true;
                        for (size_t i = 0; i < ta->arg_count; i++) {
                            if (!compareStructuralTypesHelper(ta->arguments[i], tb->arguments[i], change, changed, stack)) {
                                equal = false;
                                break;
                            }
                        }
                    }
                    break;
                }
                case TYPE_STRUCT: {
                    TypeStruct* ta = (TypeStruct*)a;
                    TypeStruct* tb = (TypeStruct*)b;
                    if (ta->count != tb->count) {
                        equal = false;
                    } else {
                        equal = true;
                        for (size_t i = 0; i < ta->count; i++) {
                            if (ta->names[i] != tb->names[i]) {
                                equal = false;
                                break;
                            }
                        }
                        if (equal) {
                            for (size_t i = 0; i < ta->count; i++) {
                                if (!compareStructuralTypesHelper(ta->types[i], tb->types[i], change, changed, stack)) {
                                    equal = false;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                case TYPE_REFERENCE: {
                    TypeReference* ta = (TypeReference*)a;
                    TypeReference* tb = (TypeReference*)b;
                    if (ta->binding->type == tb->binding->type) {
                        equal = true;
                    } else {
                        DoubleTypeReferenceStack elem = {
                            .last = stack, .types = { a, b }
                        };
                        DoubleTypeReferenceStack* cur = stack;
                        while (cur != NULL) {
                            if (cur->types[0] != a || cur->types[1] != b) {
                                cur = cur->last;
                            } else {
                                equal = true;
                                break;
                            }
                        }
                        if (!equal) {
                            equal = compareStructuralTypesHelper(ta->binding->type, tb->binding->type, change, changed, &elem);
                        }
                    }
                    break;
                }
            }
        }
        if (equal) {
            getEquivType(a)->equiv = getEquivType(b);
        }
        return equal;
    }
}

bool compareStructuralTypes(Type* a, Type* b) {
    return compareStructuralTypesHelper(a, b, false, NULL, NULL);
}

bool assertStructuralTypesEquality(Type* a, Type* b, bool* changed) {
    return compareStructuralTypesHelper(a, b, true, changed, NULL);
}

