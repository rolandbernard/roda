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

#define TYPE_BASE   \
    TypeKind kind;

typedef struct {
    TYPE_BASE
} Type;

typedef struct {
    TYPE_BASE
    size_t size;
} TypeSizedPrimitive;

typedef struct {
    TYPE_BASE
    const Type* base;
} TypePointer;

typedef struct {
    TYPE_BASE
    const Type* base;
    size_t size;
} TypeArray;

typedef struct {
    TYPE_BASE
    const Type* ret_type;
    const Type** arguments;
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

const Type* createUnsizedPrimitiveType(TypeContext* cxt, TypeKind kind);

const TypeSizedPrimitive* createSizedPrimitiveType(TypeContext* cxt, TypeKind kind, size_t size);

const TypePointer* createPointerType(TypeContext* cxt, const Type* base);

const TypeArray* createArrayType(TypeContext* cxt, const Type* base, size_t size);

const TypeFunction* createFunctionType(TypeContext* cxt, const Type* ret_type, size_t arg_count, const Type** arguments);

const TypeReference* createTypeReference(TypeContext* cxt, struct SymbolType* binding);

String buildTypeName(const Type* type);

const TypeSizedPrimitive* isSignedIntegerType(const Type* type);

const TypeSizedPrimitive* isUnsignedIntegerType(const Type* type);

const TypeSizedPrimitive* isIntegerType(const Type* type);

const TypeSizedPrimitive* isFloatType(const Type* type);

const TypeSizedPrimitive* isDoubleType(const Type* type);

const TypeSizedPrimitive* isRealType(const Type* type);

const Type* isBooleanType(const Type* type);

const TypePointer* isPointerType(const Type* type);

const TypeArray* isArrayType(const Type* type);

const TypeFunction* isFunctionType(const Type* type);

bool compareStructuralTypes(const Type* a, const Type* b);

#endif
