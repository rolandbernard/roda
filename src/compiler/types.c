
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
    type->visited = false;
    type->kind = kind;
    type->codegen = NULL;
    type->refs = NULL;
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

static void swapStructFields(size_t i, size_t j, void* cxt) {
    TypeStruct* type = (TypeStruct*)cxt;
    swap(type->names, sizeof(Symbol), i, j);
    swap(type->types, sizeof(Type*), i, j);
}

static bool compareStructFieldNames(size_t i, size_t j, void* cxt) {
    TypeStruct* type = (TypeStruct*)cxt;
    return type->names[i] <= type->names[j];
}

Type* createTypeStruct(TypeContext* cxt, struct AstNode* def, bool ordered, Symbol* names, Type** types, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (isErrorType(types[i])) {
            Type* ret = types[i];
            FREE(names);
            FREE(types);
            return ret;
        }
    }
    TypeStruct* type = NEW(TypeStruct);
    type->ordered = ordered;
    type->names = names;
    type->types = types;
    type->count = count;
    if (!ordered) {
        heapSort(count, swapStructFields, compareStructFieldNames, type);
    }
    return addAndInitType(cxt, (Type*)type, TYPE_STRUCT, def);
}

Type* createUnsureType(TypeContext* cxt, struct AstNode* def, TypeUnsureKind set, Type* fallback) {
    TypeUnsure* type = NEW(TypeUnsure);
    type->set = set;
    type->fallback = fallback;
    type->actual = NULL;
    return addAndInitType(cxt, (Type*)type, TYPE_UNSURE, def);
}

static void buildTypeNameInto(StringBuilder* dst, Type* type) {
    if (type == NULL || type->visited == true) {
        pushToStringBuilder(dst, str("_"));
    } else {
        type->visited = true;
        switch (type->kind) {
            case TYPE_ERROR: {
                pushToStringBuilder(dst, str("error"));
                break;
            }
            case TYPE_VOID: {
                pushToStringBuilder(dst, str("()"));
                break;
            }
            case TYPE_BOOL: {
                pushToStringBuilder(dst, str("bool"));
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
                pushToStringBuilder(dst, str("*"));
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
                pushToStringBuilder(dst, str("fn ("));
                for (size_t i = 0; i < t->arg_count; i++) {
                    if (i != 0) {
                        pushToStringBuilder(dst, str(", "));
                    }
                    buildTypeNameInto(dst, t->arguments[i]);
                }
                if (t->vararg) {
                    pushToStringBuilder(dst, str(", .."));
                }
                pushToStringBuilder(dst, str("): "));
                buildTypeNameInto(dst, t->ret_type);
                break;
            }
            case TYPE_REFERENCE: {
                TypeReference* t = (TypeReference*)type;
                pushToStringBuilder(dst, str(t->binding->name));
                break;
            }
            case TYPE_STRUCT: {
                TypeStruct* t = (TypeStruct*)type;
                if (t->ordered) {
                    pushToStringBuilder(dst, str("extern "));
                }
                pushToStringBuilder(dst, str("("));
                for (size_t i = 0; i < t->count; i++) {
                    if (i != 0) {
                        pushToStringBuilder(dst, str(", "));
                    }
                    pushToStringBuilder(dst, str(t->names[i]));
                    pushToStringBuilder(dst, str(" = "));
                    buildTypeNameInto(dst, t->types[i]);
                }
                pushToStringBuilder(dst, str(")"));
                break;
            }
            case TYPE_UNSURE: {
                TypeUnsure* t = (TypeUnsure*)type;
                if (t->actual != NULL) {
                    buildTypeNameInto(dst, t->actual);
                } else if (t->set == UNSURE_INTEGER) {
                    pushToStringBuilder(dst, str("{integer}"));
                } else if (t->set == UNSURE_REAL) {
                    pushToStringBuilder(dst, str("{real}"));
                } else {
                    buildTypeNameInto(dst, t->fallback);
                }
            }
        }
        type->visited = false;
    }
}

String buildTypeName(Type* type) {
    StringBuilder ret;
    initStringBuilder(&ret);
    buildTypeNameInto(&ret, type);
    return builderToString(&ret);
}

#define RETURN_VOID             \
    if (visit) {                \
        type->visited = false;  \
    }

#define RETURN(TYPE, VALUE) {   \
    TYPE result = VALUE;        \
    if (visit) {                \
        type->visited = false;  \
    }                           \
    return result;              \
}

#define STRUCTURAL_TYPE_CHECK_HELPER(TRUE, DEFAULT, ON_NULL, REC)   \
    bool visit = false;                                             \
    if (type != NULL) {                                             \
        if (type->visited) {                                        \
            DEFAULT                                                 \
        } else {                                                    \
            type->visited = true;                                   \
            visit = true;                                           \
            TRUE else if (type->kind == TYPE_REFERENCE) {           \
                TypeReference* t = (TypeReference*)type;            \
                ASSERT(t->binding != NULL);                         \
                Type* next = t->binding->type;                      \
                REC                                                 \
            } else if (type->kind == TYPE_UNSURE) {                 \
                TypeUnsure* t = (TypeUnsure*)type;                  \
                if (t->actual != NULL) {                            \
                    Type* next = t->actual;                         \
                    REC                                             \
                } else {                                            \
                    Type* next = t->fallback;                       \
                    REC                                             \
                }                                                   \
            } else {                                                \
                DEFAULT                                             \
            }                                                       \
        }                                                           \
    } else {                                                        \
        ON_NULL                                                     \
    }

Type* getTypeOfKind(Type* type, TypeKind kind) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == kind) { RETURN(Type*, type); },
        { RETURN(Type*, NULL); },
        { RETURN(Type*, NULL); },
        { RETURN(Type*, getTypeOfKind(next, kind)); }
    )
}

bool isSignedIntegerType(Type* type) {
    return getTypeOfKind(type, TYPE_INT) != NULL;
}

bool isUnsignedIntegerType(Type* type) {
    return getTypeOfKind(type, TYPE_UINT) != NULL;
}

bool isIntegerType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (
            type->kind == TYPE_INT || type->kind == TYPE_UINT
            || (type->kind == TYPE_UNSURE && ((TypeUnsure*)type)->set == UNSURE_INTEGER)
        ) {
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isIntegerType(next)) }
    )
}

bool isFloatType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 32) {
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isFloatType(next)) }
    )
}

bool isDoubleType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_REAL && ((TypeSizedPrimitive*)type)->size == 64) {
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isDoubleType(next)) }
    )
}

bool isRealType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_REAL || (type->kind == TYPE_UNSURE && ((TypeUnsure*)type)->set == UNSURE_REAL)) {
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isRealType(next)) }
    )
}

bool isNumericType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_REAL || type->kind == TYPE_INT || type->kind == TYPE_UINT) {
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isNumericType(next)) }
    )
}

bool isBooleanType(Type* type) {
    return getTypeOfKind(type, TYPE_BOOL) != NULL;
}

bool isVoidType(Type* type) {
    return getTypeOfKind(type, TYPE_VOID) != NULL;
}

bool isPointerType(Type* type) {
    return getTypeOfKind(type, TYPE_POINTER) != NULL;
}

bool isArrayType(Type* type) {
    return getTypeOfKind(type, TYPE_ARRAY) != NULL;
}

bool isFunctionType(Type* type) {
    return getTypeOfKind(type, TYPE_FUNCTION) != NULL;
}

bool isStructType(Type* type) {
    return getTypeOfKind(type, TYPE_STRUCT) != NULL;
}

bool isTypeReference(Type* type) {
    return getTypeOfKind(type, TYPE_REFERENCE) != NULL;
}

bool isUnsureType(Type* type) {
    return getTypeOfKind(type, TYPE_UNSURE) != NULL;
}

size_t getIntRealTypeSize(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_REAL || type->kind == TYPE_INT || type->kind == TYPE_UINT) {
            TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
            RETURN(size_t, t->size);
        },
        { RETURN(size_t, 0); },
        { RETURN(size_t, 0); },
        { RETURN(size_t, getIntRealTypeSize(next)) }
    )
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

bool isPartialType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (
            type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL
            || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL
        ) {
            RETURN(bool, false);
        } else if (type->kind == TYPE_POINTER) {
            TypePointer* t = (TypePointer*)type;
            RETURN(bool, isPartialType(t->base));
        } else if (type->kind == TYPE_FUNCTION) {
            TypeFunction* t = (TypeFunction*)type;
            for (size_t i = 0; i < t->arg_count; i++) {
                if (isPartialType(t->arguments[i])) {
                    RETURN(bool, true);
                }
            }
            RETURN(bool, isPartialType(t->ret_type));
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            RETURN(bool, isPartialType(array->base));
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                if (isPartialType(s->types[i])) {
                    RETURN(bool, true);
                }
            }
            RETURN(bool, false);
        },
        { RETURN(bool, false); },
        { RETURN(bool, true); },
        { RETURN(bool, isPartialType(next)) }
    )
}

bool containsErrorType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER(
        if (type->kind == TYPE_ERROR) {
            RETURN(bool, true);
        } else if (type->kind == TYPE_VOID || type->kind == TYPE_BOOL || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL) {
            RETURN(bool, false);
        } else if (type->kind == TYPE_POINTER) {
            TypePointer* t = (TypePointer*)type;
            RETURN(bool, containsErrorType(t->base));
        } else if (type->kind == TYPE_FUNCTION) {
            TypeFunction* t = (TypeFunction*)type;
            for (size_t i = 0; i < t->arg_count; i++) {
                if (containsErrorType(t->arguments[i])) {
                    RETURN(bool, true);
                }
            }
            RETURN(bool, containsErrorType(t->ret_type));
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            RETURN(bool, containsErrorType(array->base));
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                if (containsErrorType(s->types[i])) {
                    RETURN(bool, true);
                }
            }
            RETURN(bool, false);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, containsErrorType(next)) }
    )
}

void fillPartialType(Type* type, Type* with) {
    STRUCTURAL_TYPE_CHECK_HELPER( 
        if (
            type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL
            || type->kind == TYPE_INT || type->kind == TYPE_UINT || type->kind == TYPE_REAL
        ) {
            RETURN_VOID;
        } else if (type->kind == TYPE_POINTER) {
            TypePointer* t = (TypePointer*)type;
            fillPartialType(t->base, with);
            RETURN_VOID;
        } else if (type->kind == TYPE_FUNCTION) {
            TypeFunction* t = (TypeFunction*)type;
            for (size_t i = 0; i < t->arg_count; i++) {
                fillPartialType(t->arguments[i], with);
            }
            fillPartialType(t->ret_type, with);
            RETURN_VOID;
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            fillPartialType(array->base, with);
            RETURN_VOID;
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                fillPartialType(s->types[i], with);
            }
            RETURN_VOID;
        } else if (type->kind == TYPE_UNSURE) {
            TypeUnsure* t = (TypeUnsure*)type;
            if (t->actual != NULL) {
                fillPartialType(t->actual, with);
            } else if (t->fallback != NULL) {
                fillPartialType(t->fallback, with);
            } else {
                t->actual = with;
            }
            RETURN_VOID;
        },
        { RETURN_VOID; },
        { RETURN_VOID; },
        {
            fillPartialType(next, with);
            RETURN_VOID;
        }
    )
}

AstNode* getTypeReason(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER( 
        if (type->kind != TYPE_UNSURE || ((TypeUnsure*)type)->actual == NULL) {
            RETURN(AstNode*, type->def);
        },
        { RETURN(AstNode*, NULL); },
        { RETURN(AstNode*, NULL); },
        { RETURN(AstNode*, getTypeReason(next)); }
    )
}

bool isValidType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER( 
        if (
            type->kind == TYPE_ERROR || type->kind == TYPE_VOID || type->kind == TYPE_BOOL || type->kind == TYPE_INT
            || type->kind == TYPE_UINT || type->kind == TYPE_REAL || type->kind == TYPE_POINTER
        ) {
            RETURN(bool, true);
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            RETURN(bool, isValidType(array->base));
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                if (!isValidType(s->types[i])) {
                    RETURN(bool, false);
                }
            }
            RETURN(bool, true);
        } else if (type->kind == TYPE_FUNCTION) {
            TypeFunction* t = (TypeFunction*)type;
            for (size_t i = 0; i < t->arg_count; i++) {
                if (!isValidType(t->arguments[i])) {
                    RETURN(bool, false);
                }
            }
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isValidType(next)); }
    )
}

bool isEffectivelyVoidType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER( 
        if (type->kind == TYPE_VOID) {
            RETURN(bool, true);
        } else if (
            type->kind == TYPE_ERROR || type->kind == TYPE_BOOL || type->kind == TYPE_INT || type->kind == TYPE_UINT
            || type->kind == TYPE_REAL || type->kind == TYPE_POINTER || type->kind == TYPE_FUNCTION
        ) {
            RETURN(bool, false);
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            RETURN(bool, isEffectivelyVoidType(array->base));
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                if (!isEffectivelyVoidType(s->types[i])) {
                    RETURN(bool, false);
                }
            }
            RETURN(bool, true);
        },
        { RETURN(bool, true); },
        { RETURN(bool, false); },
        { RETURN(bool, isEffectivelyVoidType(next)); }
    )
}

bool isSizedType(Type* type) {
    STRUCTURAL_TYPE_CHECK_HELPER( 
        if (type->kind == TYPE_VOID) {
            RETURN(bool, true);
        } else if (type->kind == TYPE_FUNCTION) {
            RETURN(bool, false);
        } else if (
            type->kind == TYPE_ERROR || type->kind == TYPE_BOOL || type->kind == TYPE_INT
            || type->kind == TYPE_UINT || type->kind == TYPE_REAL || type->kind == TYPE_POINTER
        ) {
            RETURN(bool, true);
        } else if (type->kind == TYPE_ARRAY) {
            TypeArray* array = (TypeArray*)type;
            RETURN(bool, isSizedType(array->base));
        } else if (type->kind == TYPE_STRUCT) {
            TypeStruct* s = (TypeStruct*)type;
            for (size_t i = 0; i < s->count; i++) {
                if (!isSizedType(s->types[i])) {
                    RETURN(bool, false);
                }
            }
            RETURN(bool, true);
        },
        { RETURN(bool, false); },
        { RETURN(bool, false); },
        { RETURN(bool, isSizedType(next)); }
    )
}

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

static bool isSubsetOfUnsure(TypeUnsureKind set, Type* type) {
    if (set == UNSURE_INTEGER) {
        return isIntegerType(type);
    } else if (set == UNSURE_REAL) {
        return isRealType(type);
    } else {
        return true;
    }
}

static bool compareStructuralTypesHelper(Type* a, Type* b, TypeUnsure** changed, DoubleTypeReferenceStack* stack) {
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
        stack = &elem;
        if (!equal) {
            if (a->kind != b->kind) {
                if (a->kind == TYPE_REFERENCE) {
                    TypeReference* t = (TypeReference*)a;
                    equal = compareStructuralTypesHelper(t->binding->type, b, changed, stack);
                } else if (b->kind == TYPE_REFERENCE) {
                    TypeReference* t = (TypeReference*)b;
                    equal = compareStructuralTypesHelper(a, t->binding->type, changed, stack);
                } else if (a->kind == TYPE_UNSURE) {
                    TypeUnsure* t = (TypeUnsure*)a;
                    if (t->actual != NULL) {
                        equal = compareStructuralTypesHelper(t->actual, b, changed, stack);
                    } else if (changed != NULL && isSubsetOfUnsure(t->set, b)) {
                        t->actual = b;
                        t->changed_next = *changed;
                        *changed = t;
                        equal = true;
                    } else {
                        equal = compareStructuralTypesHelper(t->fallback, b, changed, stack);
                    }
                } else if (b->kind == TYPE_UNSURE) {
                    TypeUnsure* t = (TypeUnsure*)b;
                    if (t->actual != NULL) {
                        equal = compareStructuralTypesHelper(a, t->actual, changed, stack);
                    } else if (changed != NULL && isSubsetOfUnsure(t->set, a)) {
                        t->actual = a;
                        t->changed_next = *changed;
                        *changed = t;
                        equal = true;
                    } else {
                        equal = compareStructuralTypesHelper(a, t->fallback, changed, stack);
                    }
                } else {
                    equal = false;
                }
            } else {
                switch (a->kind) {
                    case TYPE_UNSURE: {
                        TypeUnsure* ta = (TypeUnsure*)a;
                        TypeUnsure* tb = (TypeUnsure*)b;
                        if (changed != NULL && ta->actual == NULL && isSubsetOfUnsure(ta->set, b)) {
                            ta->actual = b;
                            ta->changed_next = *changed;
                            *changed = ta;
                            equal = true;
                        } else if (changed != NULL && tb->actual == NULL && isSubsetOfUnsure(tb->set, a)) {
                            tb->actual = a;
                            tb->changed_next = *changed;
                            *changed = tb;
                            equal = true;
                        } else if (ta->actual != NULL) {
                            equal = compareStructuralTypesHelper(ta->actual, b, changed, stack);
                        } else if (tb->actual != NULL) {
                            equal = compareStructuralTypesHelper(a, tb->actual, changed, stack);
                        } else {
                            equal = compareStructuralTypesHelper(ta->fallback, tb->fallback, changed, stack);
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
                        equal = compareStructuralTypesHelper(ta->base, tb->base, changed, stack);
                        break;
                    }
                    case TYPE_ARRAY: {
                        TypeArray* ta = (TypeArray*)a;
                        TypeArray* tb = (TypeArray*)b;
                        equal = ta->size == tb->size && compareStructuralTypesHelper(ta->base, tb->base, changed, stack);
                        break;
                    }
                    case TYPE_FUNCTION: {
                        TypeFunction* ta = (TypeFunction*)a;
                        TypeFunction* tb = (TypeFunction*)b;
                        if (
                            ta->arg_count != tb->arg_count || ta->vararg != tb->vararg
                            || !compareStructuralTypesHelper(ta->ret_type, tb->ret_type, changed, stack)
                        ) {
                            equal = false;
                        } else {
                            equal = true;
                            for (size_t i = 0; i < ta->arg_count; i++) {
                                if (!compareStructuralTypesHelper(ta->arguments[i], tb->arguments[i], changed, stack)) {
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
                        if (ta->ordered != tb->ordered || ta->count != tb->count) {
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
                                    if (!compareStructuralTypesHelper(ta->types[i], tb->types[i], changed, stack)) {
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
                        equal = compareStructuralTypesHelper(ta->binding->type, tb->binding->type, changed, stack);
                        break;
                    }
                }
            }
            if (equal) {
                getEquivType(a)->equiv = getEquivType(b);
            }
        }
        return equal;
    }
}

bool compareStructuralTypes(Type* a, Type* b) {
    return compareStructuralTypesHelper(a, b, NULL, NULL);
}

bool assertStructuralTypesEquality(Type* a, Type* b, TypeUnsure** changed) {
    return compareStructuralTypesHelper(a, b, changed, NULL);
}

