
#include "errors/fatalerror.h"
#include "codegen/llvm/gentype.h"

#include "codegen/llvm/genconst.h"

LLVMValueRef generateLlvmConstValue(LlvmCodegenContext* context, ConstValue value) {
    LLVMTypeRef llvm_type = generateLlvmType(context, value.type);
    if (isSignedIntegerType(value.type)) {
        return LLVMConstInt(llvm_type, value.sint, true);
    } else if (isUnsignedIntegerType(value.type)) {
        return LLVMConstInt(llvm_type, value.uint, false);
    } else if (isBooleanType(value.type)) {
        return LLVMConstInt(llvm_type, value.boolean, true);
    } else if (isFloatType(value.type)) {
        return LLVMConstReal(llvm_type, value.f32);
    } else if (isDoubleType(value.type)) {
        return LLVMConstReal(llvm_type, value.f64);
    } else {
        UNREACHABLE("Unhandled constant value");
    }
}

