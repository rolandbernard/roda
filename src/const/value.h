#ifndef _RODA_CONST_VALUE_H_
#define _RODA_CONST_VALUE_H_

#include <stdint.h>

#include "types/types.h"
#include "const/fixedint.h"

typedef enum {
    CONST_VOID,
    CONST_INT,
    CONST_UINT,
    CONST_DOUBLE,
    CONST_FLOAT,
    CONST_BOOL,
    CONST_ARRAY,
    CONST_STRUCT,
    CONST_TUPLE,
} ConstValueKind;

#define CONST_VALUE_BASE    \
    ConstValueKind kind;    \
    Type* type;             \
    size_t refs;

typedef struct {
    CONST_VALUE_BASE
} ConstValue;

typedef struct {
    CONST_VALUE_BASE
    FixedInt* val;
} ConstValueInt;

typedef struct {
    CONST_VALUE_BASE
    double val;
} ConstValueDouble;

typedef struct {
    CONST_VALUE_BASE
    float val;
} ConstValueFloat;

typedef struct {
    CONST_VALUE_BASE
    bool val;
} ConstValueBool;

typedef struct {
    CONST_VALUE_BASE
    size_t count;
    ConstValue** values;
} ConstValueList;

typedef struct {
    CONST_VALUE_BASE
    size_t count;
    Symbol* names;
    ConstValue** values;
} ConstValueStruct;

ConstValue* createConstVoid(Type* type);

ConstValue* createConstInt(Type* type, FixedInt* value);

ConstValue* createConstUint(Type* type, FixedInt* value);

ConstValue* createConstDouble(Type* type, double value);

ConstValue* createConstFloat(Type* type, float value);

ConstValue* createConstBool(Type* type, bool value);

ConstValue* createConstArray(Type* type, ConstValue** values, size_t count);

ConstValue* createConstTuple(Type* type, ConstValue** values, size_t count);

ConstValue* createConstStruct(Type* type, Symbol* names, ConstValue** values, size_t count);

ConstValue* copyConstValue(ConstValue* value);

void freeConstValue(ConstValue* value);

ConstValue* getValueFromConstStruct(ConstValueStruct* value, Symbol name);

#endif
