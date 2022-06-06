#ifndef _COMPILER_TYPES_H_
#define _COMPILER_TYPES_H_

#include <stddef.h>
#include <stdbool.h>

#include "text/string.h"

typedef enum {
    TYPE_ERROR,
    TYPE_NEVER,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_REAL,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_REFERENCE,
} TypeKind;

#define TYPE_BASE       \
    TypeKind kind;      \
    struct Type* equivalent;

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
} TypeFunction;

typedef struct {
    TYPE_BASE
    const struct SymbolType* binding;
} TypeReference;

typedef struct {
    Type** types;
    size_t count;
    size_t capacity;
} TypeContext;

void initTypeContext(TypeContext* cxt);

void deinitTypeContext(TypeContext* cxt);

Type* createUnsizedPrimitiveType(TypeContext* cxt, TypeKind kind);

TypeSizedPrimitive* createSizedPrimitiveType(TypeContext* cxt, TypeKind kind, size_t size);

TypePointer* createPointerType(TypeContext* cxt, Type* base);

TypeArray* createArrayType(TypeContext* cxt, Type* base, size_t size);

TypeFunction* createFunctionType(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments);

TypeReference* createTypeReference(TypeContext* cxt, struct SymbolType* binding);

String buildTypeName(const Type* type);

TypeSizedPrimitive* isSignedIntegerType(Type* type);

TypeSizedPrimitive* isUnsignedIntegerType(Type* type);

TypeSizedPrimitive* isIntegerType(Type* type);

TypeSizedPrimitive* isFloatType(Type* type);

TypeSizedPrimitive* isDoubleType(Type* type);

TypeSizedPrimitive* isRealType(Type* type);

Type* isBooleanType(Type* type);

TypePointer* isPointerType(Type* type);

TypeArray* isArrayType(Type* type);

TypeFunction* isFunctionType(Type* type);

bool isErrorType(Type* type);

bool compareStructuralTypes(Type* a, Type* b);

#endif
