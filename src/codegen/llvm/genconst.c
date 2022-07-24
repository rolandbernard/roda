
#include "util/alloc.h"
#include "errors/fatalerror.h"
#include "codegen/llvm/gentype.h"

#include "codegen/llvm/genconst.h"

LLVMValueRef generateLlvmConstValue(LlvmCodegenContext* context, ConstValue* value) {
    LLVMTypeRef llvm_type = generateLlvmType(context, value->type);
    switch (value->kind) {
        case CONST_VOID:
            return LLVMConstStructInContext(context->llvm_cxt, NULL, 0, false);
        case CONST_DOUBLE: {
            ConstValueDouble* v = (ConstValueDouble*)value;
            return LLVMConstReal(llvm_type, v->val);
        }
        case CONST_FLOAT: {
            ConstValueFloat* v = (ConstValueFloat*)value;
            return LLVMConstReal(llvm_type, v->val);
        }
        case CONST_BOOL: {
            ConstValueBool* v = (ConstValueBool*)value;
            return LLVMConstInt(llvm_type, v->val ? 1 : 0, false);
        }
        case CONST_INT: {
            ConstValueInt* v = (ConstValueInt*)value;
            size_t length = 0;
            uint64_t* words = convertTo64BitWordsSignExtend(v->val, &length);
            LLVMValueRef res = LLVMConstIntOfArbitraryPrecision(llvm_type, length, words);
            FREE(words);
            return res;
        }
        case CONST_UINT: {
            ConstValueInt* v = (ConstValueInt*)value;
            size_t length = 0;
            uint64_t* words = convertTo64BitWordsZeroExtend(v->val, &length);
            LLVMValueRef res = LLVMConstIntOfArbitraryPrecision(llvm_type, length, words);
            FREE(words);
            return res;
        }
        case CONST_BIG_INT: {
            ConstValueBigInt* v = (ConstValueBigInt*)value;
            FixedInt* fixed = createFixedIntFromBigInt(LLVMGetIntTypeWidth(llvm_type), v->val);
            size_t length = 0;
            uint64_t* words = convertTo64BitWordsZeroExtend(fixed, &length);
            freeFixedInt(fixed);
            LLVMValueRef res = LLVMConstIntOfArbitraryPrecision(llvm_type, length, words);
            FREE(words);
            return res;
        }
        case CONST_ARRAY: {
            ConstValueList* v = (ConstValueList*)value;
            LLVMValueRef* elems = ALLOC(LLVMValueRef, v->count);
            for (size_t i = 0; i < v->count; i++) {
                elems[i] = generateLlvmConstValue(context, v->values[i]);
            }
            LLVMValueRef res = LLVMConstArray(LLVMGetElementType(llvm_type), elems, v->count);
            FREE(elems);
            return res;
        }
        case CONST_TUPLE: {
            ConstValueList* v = (ConstValueList*)value;
            LLVMValueRef* elems = ALLOC(LLVMValueRef, v->count);
            for (size_t i = 0; i < v->count; i++) {
                size_t idx = CODEGEN(v->type)->struct_mapping[i];
                elems[idx] = generateLlvmConstValue(context, v->values[i]);
            }
            LLVMValueRef res = LLVMConstStructInContext(context->llvm_cxt, elems, v->count, false);
            FREE(elems);
            return res;
        }
        case CONST_STRUCT: {
            ConstValueStruct* v = (ConstValueStruct*)value;
            TypeStruct* type = (TypeStruct*)getTypeOfKind(v->type, TYPE_STRUCT);
            LLVMValueRef* elems = ALLOC(LLVMValueRef, v->count);
            for (size_t i = 0; i < v->count; i++) {
                size_t idx = lookupIndexOfStructField(type, v->names[i]);
                idx = CODEGEN(v->type)->struct_mapping[idx];
                elems[idx] = generateLlvmConstValue(context, v->values[i]);
            }
            LLVMValueRef res = LLVMConstStructInContext(context->llvm_cxt, elems, v->count, false);
            FREE(elems);
            return res;
        }
    }
    UNREACHABLE("unknown const value");
}

