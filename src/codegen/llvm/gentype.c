
#include <llvm-c/Core.h>
#include <string.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/sort.h"

#include "codegen/llvm/gentype.h"

typedef struct {
    LlvmCodegenContext* context;
    TypeStruct* struc;
    LLVMTypeRef* types;
    size_t* mapping;
} StructSortContext;

static void structSortSwap(size_t i, size_t j, void* cxt) {
    StructSortContext* s = (StructSortContext*)cxt;
    swap(s->types, sizeof(LLVMTypeRef), i, j);
    swap(s->mapping, sizeof(size_t), i, j);
}

static bool structSortCompare(size_t i, size_t j, void* cxt) {
    StructSortContext* s = (StructSortContext*)cxt;
    size_t size_i = LLVMABISizeOfType(s->context->target_data, s->types[i]);
    size_t size_j = LLVMABISizeOfType(s->context->target_data, s->types[j]);
    if (size_i != size_j) {
        return size_i >= size_j;
    } else {
        return s->struc->names[i] <= s->struc->names[j];
    }
}

static void invertStructFieldMapping(size_t* mapping, size_t count) {
    size_t tmp[count];
    memcpy(tmp, mapping, count * sizeof(size_t));
    for (size_t i = 0; i < count; i++) {
        mapping[tmp[i]] = i;
    }
}

LLVMTypeRef generateLlvmType(LlvmCodegenContext* context, Type* type) {
    if (CODEGEN(type)->type == NULL) {
        CODEGEN(type)->type = context->opaque_type;
        LLVMTypeRef result = NULL;
        switch (type->kind) {
            case TYPE_ERROR:
                UNREACHABLE();
            case TYPE_STRUCT: {
                StructSortContext cxt;
                cxt.context = context;
                cxt.struc = (TypeStruct*)type;
                cxt.types = ALLOC(LLVMTypeRef, cxt.struc->count);
                cxt.mapping = ALLOC(size_t, cxt.struc->count);
                for (size_t i = 0; i < cxt.struc->count; i++) {
                    cxt.types[i] = generateLlvmType(context, cxt.struc->types[i]);
                    cxt.mapping[i] = i;
                }
                if (!cxt.struc->ordered) {
                    heapSort(cxt.struc->count, structSortSwap, structSortCompare, &cxt);
                }
                invertStructFieldMapping(cxt.mapping, cxt.struc->count);
                CODEGEN(type)->struct_mapping = cxt.mapping;
                result = LLVMStructTypeInContext(context->llvm_cxt, cxt.types, cxt.struc->count, false);
                FREE(cxt.types);
                break;
            }
            case TYPE_VOID:
                result = LLVMStructTypeInContext(context->llvm_cxt, NULL, 0, false);
                break;
            case TYPE_BOOL:
                result = LLVMInt1TypeInContext(context->llvm_cxt);
                break;
            case TYPE_INT:
            case TYPE_UINT: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                if (t->size != SIZE_SIZE) {
                    result = LLVMIntTypeInContext(context->llvm_cxt, t->size);
                } else {
                    result = LLVMIntPtrTypeInContext(context->llvm_cxt, context->target_data);
                }
                break;
            }
            case TYPE_REAL: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                if (t->size == 32) {
                    result = LLVMFloatTypeInContext(context->llvm_cxt);
                } else if (t->size == 64) {
                    result = LLVMDoubleTypeInContext(context->llvm_cxt);
                } else {
                    UNREACHABLE();
                }
                break;
            }
            case TYPE_POINTER: {
                TypePointer* t = (TypePointer*)type;
                result = LLVMPointerType(generateLlvmType(context, t->base), 0);
                break;
            }
            case TYPE_ARRAY: {
                TypeArray* t = (TypeArray*)type;
                result = LLVMArrayType(generateLlvmType(context, t->base), t->size);
                break;
            }
            case TYPE_FUNCTION: {
                TypeFunction* t = (TypeFunction*)type;
                LLVMTypeRef ret_type = generateLlvmType(context, t->ret_type);
                LLVMTypeRef* param_types = ALLOC(LLVMTypeRef, t->arg_count);
                for (size_t i = 0; i < t->arg_count; i++) {
                    param_types[i] = generateLlvmType(context, t->arguments[i]);
                }
                result = LLVMFunctionType(ret_type, param_types, t->arg_count, t->vararg);
                FREE(param_types);
                break;
            }
            case TYPE_REFERENCE: {
                TypeReference* t = (TypeReference*)type;
                result = generateLlvmType(context, t->binding->type);
                break;
            }
            case TYPE_UNSURE: {
                TypeUnsure* t = (TypeUnsure*)type;
                if (t->actual != NULL) {
                    result = generateLlvmType(context, t->actual);
                } else {
                    result = generateLlvmType(context, t->fallback);
                }
                break;
            }
        }
        CODEGEN(type)->type = result;
    }
    return CODEGEN(type)->type;
}

