
#include "util/alloc.h"

#include "const/value.h"

static void initConstValue(ConstValue* value, ConstValueKind kind, Type* type) {
    value->kind = kind;
    value->type = type;
    value->refs = 1;
}

ConstValue* createConstVoid(Type* type) {
    ConstValue* res = NEW(ConstValue);
    initConstValue(res, CONST_VOID, type);
    return res;
}

ConstValue* createConstInt(Type* type, FixedInt* value) {
    ConstValueInt* res = NEW(ConstValueInt);
    initConstValue((ConstValue*)res, CONST_INT, type);
    res->val = value;
    return (ConstValue*)res;
}

ConstValue* createConstUint(Type* type, FixedInt* value) {
    ConstValueInt* res = NEW(ConstValueInt);
    initConstValue((ConstValue*)res, CONST_UINT, type);
    res->val = value;
    return (ConstValue*)res;
}

ConstValue* createConstDouble(Type* type, double value) {
    ConstValueDouble* res = NEW(ConstValueDouble);
    initConstValue((ConstValue*)res, CONST_DOUBLE, type);
    res->val = value;
    return (ConstValue*)res;
}

ConstValue* createConstFloat(Type* type, float value) {
    ConstValueFloat* res = NEW(ConstValueFloat);
    initConstValue((ConstValue*)res, CONST_FLOAT, type);
    res->val = value;
    return (ConstValue*)res;
}

ConstValue* createConstBool(Type* type, bool value) {
    ConstValueBool* res = NEW(ConstValueBool);
    initConstValue((ConstValue*)res, CONST_BOOL, type);
    res->val = value;
    return (ConstValue*)res;
}

ConstValue* createConstArray(Type* type, ConstValue** values, size_t count) {
    ConstValueList* res = NEW(ConstValueList);
    initConstValue((ConstValue*)res, CONST_ARRAY, type);
    res->count = count;
    res->values = values;
    return (ConstValue*)res;
}

ConstValue* createConstTuple(Type* type, ConstValue** values, size_t count) {
    ConstValueList* res = NEW(ConstValueList);
    initConstValue((ConstValue*)res, CONST_TUPLE, type);
    res->count = count;
    res->values = values;
    return (ConstValue*)res;
}

ConstValue* createConstStruct(Type* type, Symbol* names, ConstValue** values, size_t count) {
    ConstValueStruct* res = NEW(ConstValueStruct);
    initConstValue((ConstValue*)res, CONST_STRUCT, type);
    res->count = count;
    res->names = names;
    res->values = values;
    return (ConstValue*)res;
}

ConstValue* copyConstValue(ConstValue* value) {
    value->refs++;
    return value;
}

void freeConstValue(ConstValue* value) {
    if (value != NULL) {
        value->refs--;
        if (value->refs == 0) {
            switch (value->kind) {
                case CONST_VOID:
                case CONST_DOUBLE:
                case CONST_FLOAT:
                case CONST_BOOL:
                    break;
                case CONST_INT:
                case CONST_UINT: {
                    ConstValueInt* v = (ConstValueInt*)value;
                    freeFixedInt(v->val);
                    break;
                }
                case CONST_ARRAY:
                case CONST_TUPLE: {
                    ConstValueList* v = (ConstValueList*)value;
                    for (size_t i = 0; i < v->count; i++) {
                        freeConstValue(v->values[i]);
                    }
                    FREE(v->values);
                    break;
                }
                case CONST_STRUCT: {
                    ConstValueStruct* v = (ConstValueStruct*)value;
                    for (size_t i = 0; i < v->count; i++) {
                        freeConstValue(v->values[i]);
                    }
                    FREE(v->names);
                    FREE(v->values);
                    break;
                }
            }
            FREE(value);
        }
    }
}

ConstValue* getValueFromConstStruct(ConstValueStruct* value, Symbol name) {
    for (size_t i = 0; i < value->count; i++) {
        if (value->names[i] == name) {
            return value->values[i];
        }
    }
    return NULL;
}

