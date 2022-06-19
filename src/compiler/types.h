#ifndef _COMPILER_TYPES_H_
#define _COMPILER_TYPES_H_

#include <stddef.h>
#include <stdbool.h>

#include "text/symbol.h"
#include "text/string.h"

typedef enum {
    TYPE_ERROR,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_REAL,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_REFERENCE,
    TYPE_STRUCT,
    TYPE_UNSURE,
} TypeKind;

#define TYPE_BASE       \
    TypeKind kind;      \
    struct Type* equiv; \
    void* codegen;

typedef struct Type {
    TYPE_BASE
} Type;

typedef struct {
    TYPE_BASE
    size_t size;
} TypeSizedPrimitive;

typedef struct {
    TYPE_BASE
    Type* base;
} TypePointer;

typedef struct {
    TYPE_BASE
    Type* base;
    size_t size;
} TypeArray;

typedef struct {
    TYPE_BASE
    Type* ret_type;
    Type** arguments;
    size_t arg_count;
    bool vararg;
} TypeFunction;

typedef struct {
    TYPE_BASE
    struct SymbolType* binding;
} TypeReference;

typedef struct {
    TYPE_BASE
    Symbol* names;
    Type** types;
    size_t count;
} TypeStruct;

typedef struct {
    TYPE_BASE
    Type* fallback;
    Type* actual;
} TypeUnsure;

typedef struct {
    Type** types;
    size_t count;
    size_t capacity;
} TypeContext;

void initTypeContext(TypeContext* cxt);

void deinitTypeContext(TypeContext* cxt);

Type* createUnsizedPrimitiveType(TypeContext* cxt, TypeKind kind);

Type* createSizedPrimitiveType(TypeContext* cxt, TypeKind kind, size_t size);

Type* createPointerType(TypeContext* cxt, Type* base);

Type* createArrayType(TypeContext* cxt, Type* base, size_t size);

Type* createFunctionType(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments, bool vararg);

Type* createTypeReference(TypeContext* cxt, struct SymbolType* binding);

Type* createTypeStruct(TypeContext* cxt, Symbol* name, Type** types, size_t count);

Type* createUnsureType(TypeContext* cxt, Type* fallback);

String buildTypeName(const Type* type);

TypeSizedPrimitive* isSignedIntegerType(Type* type);

TypeSizedPrimitive* isUnsignedIntegerType(Type* type);

TypeSizedPrimitive* isIntegerType(Type* type);

TypeSizedPrimitive* isFloatType(Type* type);

TypeSizedPrimitive* isDoubleType(Type* type);

TypeSizedPrimitive* isRealType(Type* type);

TypeSizedPrimitive* isNumericType(Type* type);

Type* isBooleanType(Type* type);

Type* isVoidType(Type* type);

TypePointer* isPointerType(Type* type);

TypeArray* isArrayType(Type* type);

TypeFunction* isFunctionType(Type* type);

TypeStruct* isStructType(Type* type);

TypeReference* isTypeReference(Type* type);

TypeUnsure* isUnsureType(Type* type);

size_t lookupIndexOfStructField(TypeStruct* type, Symbol name);

bool isPartialType(Type* type);

void fillPartialType(Type* type, Type* with);

bool isValidType(Type* type);

bool isSizedType(Type* type);

bool isEffectivelyVoidType(Type* type);

bool isErrorType(Type* type);

bool containsErrorType(Type* type);

bool compareStructuralTypes(Type* a, Type* b);

bool assertStructuralTypesEquality(Type* a, Type* b, bool* changed);

#endif
