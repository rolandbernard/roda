
#include <llvm-c/Core.h>

#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/sort.h"

#include "codegen/llvm/gentype.h"

typedef struct {
    LlvmCodegenContext* context;
    TypeStruct* struc;
    LLVMTypeRef* types;
} StructSortContext;

static void structSortSwap(size_t i, size_t j, void* cxt) {
    StructSortContext* s = (StructSortContext*)cxt;
    swap(s->types, sizeof(LLVMTypeRef), i, j);
    swap(s->struc->names, sizeof(Symbol), i, j);
    swap(s->struc->types, sizeof(Type*), i, j);
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

typedef struct TypeReferenceStack {
    struct TypeReferenceStack* last;
    const SymbolType* binding;
} TypeReferenceStack;

static LLVMTypeRef generateLlvmTypeHelper(LlvmCodegenContext* context, Type* type, TypeReferenceStack* stack) {
    if (CODEGEN(type)->type == NULL) {
        LLVMTypeRef result = NULL;
        switch (type->kind) {
            case TYPE_ERROR:
                UNREACHABLE();
            case TYPE_STRUCT: {
                StructSortContext cxt;
                cxt.context = context;
                cxt.struc = (TypeStruct*)type;
                cxt.types = ALLOC(LLVMTypeRef, cxt.struc->count);
                for (size_t i = 0; i < cxt.struc->count; i++) {
                    cxt.types[i] = generateLlvmTypeHelper(context, cxt.struc->types[i], stack);
                }
                heapSort(cxt.struc->count, structSortSwap, structSortCompare, &cxt);
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
                result = LLVMIntTypeInContext(context->llvm_cxt, t->size);
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
                result = LLVMPointerType(generateLlvmTypeHelper(context, t->base, stack), 0);
                break;
            }
            case TYPE_ARRAY: {
                TypeArray* t = (TypeArray*)type;
                result = LLVMArrayType(generateLlvmTypeHelper(context, t->base, stack), t->size);
                break;
            }
            case TYPE_FUNCTION: {
                TypeFunction* t = (TypeFunction*)type;
                LLVMTypeRef ret_type = generateLlvmTypeHelper(context, t->ret_type, stack);
                LLVMTypeRef* param_types = ALLOC(LLVMTypeRef, t->arg_count);
                for (size_t i = 0; i < t->arg_count; i++) {
                    param_types[i] = generateLlvmTypeHelper(context, t->arguments[i], stack);
                }
                result = LLVMFunctionType(ret_type, param_types, t->arg_count, t->vararg);
                FREE(param_types);
                break;
            }
            case TYPE_REFERENCE: {
                TypeReference* t = (TypeReference*)type;
                TypeReferenceStack elem = { .last = stack, .binding = t->binding };
                TypeReferenceStack* cur = stack;
                while (cur != NULL) {
                    if (cur->binding != elem.binding) {
                        cur = cur->last;
                    } else {
                        return LLVMStructCreateNamed(context->llvm_cxt, t->binding->name);
                    }
                }
                result = generateLlvmTypeHelper(context, t->binding->type, &elem);
                break;
            }
            case TYPE_UNSURE: {
                TypeUnsure* t = (TypeUnsure*)type;
                if (t->actual != NULL) {
                    result = generateLlvmTypeHelper(context, t->actual, stack);
                } else {
                    result = generateLlvmTypeHelper(context, t->fallback, stack);
                }
                break;
            }
        }
        CODEGEN(type)->type = result;
    }
    return CODEGEN(type)->type;
}

LLVMTypeRef generateLlvmType(LlvmCodegenContext* context, Type* type) {
    return generateLlvmTypeHelper(context, type, NULL);
}

