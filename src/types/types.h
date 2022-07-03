#ifndef _RODA_COMPILER_TYPES_H_
#define _RODA_COMPILER_TYPES_H_

#include <stddef.h>
#include <stdbool.h>

#include "text/symbol.h"
#include "text/string.h"

#define SIZE_SIZE ((size_t)-1)

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
    TYPE_TUPLE,
} TypeKind;

#define TYPE_BASE           \
    TypeKind kind;          \
    bool visited;           \
    struct Type* next;      \
    struct Type* equiv;     \
    struct AstNode* def;    \
    struct AstNode* refs;   \
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
    bool ordered;
    Symbol* names;
    Type** types;
    size_t count;
} TypeStruct;

typedef struct {
    TYPE_BASE
    Type** types;
    size_t count;
} TypeTuple;

typedef enum {
    UNSURE_ANY,
    UNSURE_INTEGER,
    UNSURE_REAL,
} TypeUnsureKind;

typedef struct TypeUnsure {
    TYPE_BASE
    struct TypeUnsure* changed_next;
    TypeUnsureKind set;
    Type* fallback;
    Type* actual;
} TypeUnsure;

typedef struct {
    Type* types;
    Type* error;
} TypeContext;

void initTypeContext(TypeContext* cxt);

void deinitTypeContext(TypeContext* cxt);

Type* getErrorType(TypeContext* cxt);

Type* createUnsizedPrimitiveType(TypeContext* cxt, struct AstNode* def, TypeKind kind);

Type* createSizedPrimitiveType(TypeContext* cxt, struct AstNode* def, TypeKind kind, size_t size);

Type* createPointerType(TypeContext* cxt, struct AstNode* def, Type* base);

Type* createArrayType(TypeContext* cxt, struct AstNode* def, Type* base, size_t size);

Type* createFunctionType(TypeContext* cxt, struct AstNode* def, Type* ret_type, size_t arg_count, Type** arguments, bool vararg);

Type* createTypeReference(TypeContext* cxt, struct AstNode* def, struct SymbolType* binding);

Type* createTypeStruct(TypeContext* cxt, struct AstNode* def, bool ordered, Symbol* name, Type** types, size_t count);

Type* createTypeTuple(TypeContext* cxt, struct AstNode* def, Type** types, size_t count);

size_t lookupIndexOfStructField(TypeStruct* type, Symbol name);

Type* createUnsureType(TypeContext* cxt, struct AstNode* def, TypeUnsureKind set, Type* fallback);

String buildTypeName(Type* type);

struct AstNode* getTypeReason(Type* type);

Type* getTypeOfKind(Type* type, TypeKind kind);

bool isSignedIntegerType(Type* type);

bool isUnsignedIntegerType(Type* type);

bool isIntegerType(Type* type);

bool isFloatType(Type* type);

bool isDoubleType(Type* type);

bool isRealType(Type* type);

bool isNumericType(Type* type);

bool isBooleanType(Type* type);

bool isVoidType(Type* type);

bool isPointerType(Type* type);

bool isArrayType(Type* type);

bool isFunctionType(Type* type);

bool isStructType(Type* type);

bool isTupleType(Type* type);

bool isTypeReference(Type* type);

bool isUnsureType(Type* type);

bool isPartialType(Type* type);

bool isValidType(Type* type);

bool isSizedType(Type* type);

bool isEffectivelyVoidType(Type* type);

bool isErrorType(Type* type);

bool containsErrorType(Type* type);

bool compareStructuralTypes(Type* a, Type* b);

size_t getIntRealTypeSize(Type* type);

void fillPartialType(Type* type, Type* with);

bool assertStructuralTypesEquality(Type* a, Type* b, TypeUnsure** changed);

#endif
