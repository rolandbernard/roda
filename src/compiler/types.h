#ifndef _COMPILER_TYPES_H_
#define _COMPILER_TYPES_H_

#include <stddef.h>

typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_REAL,
    TYPE_POINTER,
    TYPE_ARRAY,
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
    Type** types;
    size_t count;
    size_t capacity;
} TypeContext;

void initTypeContext(TypeContext* cxt);

void deinitTypeContext(TypeContext* cxt);

Type* createUnsizedPrimitive(TypeContext* cxt, TypeKind kind);

Type* createSizedPrimitive(TypeContext* cxt, TypeKind kind, size_t size);

Type* createPointer(TypeContext* cxt, Type* base);

Type* createArray(TypeContext* cxt, Type* base, size_t size);

#endif
