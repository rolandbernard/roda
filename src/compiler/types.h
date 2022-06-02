#ifndef _COMPILER_TYPES_H_
#define _COMPILER_TYPES_H_

#include <stddef.h>

typedef enum {
    TYPE_VOID,
    TYPE_INT,
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

Type* createUnsizedPrimitive(TypeContext* cxt, TypeKind kind);

TypeSizedPrimitive* createSizedPrimitive(TypeContext* cxt, TypeKind kind, size_t size);

TypePointer* createPointer(TypeContext* cxt, Type* base);

TypeArray* createArray(TypeContext* cxt, Type* base, size_t size);

TypeFunction* createFunction(TypeContext* cxt, Type* ret_type, size_t arg_count, Type** arguments);

#endif
