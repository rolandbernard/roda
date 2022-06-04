#ifndef _COMPILER_TYPES_H_
#define _COMPILER_TYPES_H_

#include "text/string.h"

#include <stddef.h>

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
} TypeKind;

#define TYPE_BASE \
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

String buildTypeName(Type* type);

#endif
