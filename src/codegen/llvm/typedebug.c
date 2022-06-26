
#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <string.h>

#include "ast/ast.h"
#include "codegen/llvm/gentype.h"
#include "errors/fatalerror.h"
#include "util/alloc.h"
#include "util/sort.h"

#include "codegen/llvm/typedebug.h"

LLVMMetadataRef generateLlvmTypeDebugInfo(LlvmCodegenContext* context, LlvmCodegenModuleContext* data, Type* type, size_t line) {
    if (CODEGEN(type)->debug == NULL) {
        LLVMMetadataRef tmp = LLVMTemporaryMDNode(context->llvm_cxt, NULL, 0);
        CODEGEN(type)->debug = tmp;
        String name = buildTypeName(type);
        LLVMTypeRef llvm_type = generateLlvmType(context, type);
        LLVMMetadataRef result = NULL;
        switch (type->kind) {
            case TYPE_ERROR:
                UNREACHABLE();
            case TYPE_STRUCT: {
                TypeStruct* t = (TypeStruct*)type;
                LLVMMetadataRef* fields = ALLOC(LLVMMetadataRef, t->count);
                for (size_t i = 0; i < t->count; i++) {
                    LLVMTypeRef field_type = generateLlvmType(context, t->types[i]);
                    fields[i] = generateLlvmTypeDebugInfo(context, data, t->types[i], line);
                    fields[i] = LLVMDIBuilderCreateMemberType(
                        data->debug_bulder, tmp, t->names[i], strlen(t->names[i]),
                        data->file_metadata, line,
                        8 * LLVMABISizeOfType(context->target_data, field_type),
                        8 * LLVMABIAlignmentOfType(context->target_data, field_type),
                        8 * LLVMOffsetOfElement(context->target_data, llvm_type, i), 0, fields[i]
                    );
                }
                result = LLVMDIBuilderCreateStructType(
                    data->debug_bulder, data->file_metadata, name.data, name.length,
                    data->file_metadata, line,
                    8 * LLVMABISizeOfType(context->target_data, llvm_type),
                    8 * LLVMABIAlignmentOfType(context->target_data, llvm_type), 0, NULL, fields,
                    t->count, 0, NULL, NULL, 0
                );
                FREE(fields);
                break;
            }
            case TYPE_VOID:
                result = LLVMDIBuilderCreateStructType(
                    data->debug_bulder, data->file_metadata, name.data, name.length,
                    data->file_metadata, line,
                    8 * LLVMABISizeOfType(context->target_data, llvm_type),
                    8 * LLVMABIAlignmentOfType(context->target_data, llvm_type), 0, NULL, NULL, 0,
                    0, NULL, NULL, 0
                );
                break;
            case TYPE_BOOL:
                result = LLVMDIBuilderCreateBasicType(data->debug_bulder, name.data, name.length, 1, 2, 0);
                break;
            case TYPE_INT: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                result = LLVMDIBuilderCreateBasicType(data->debug_bulder, name.data, name.length, t->size, 5, 0);
                break;
            }
            case TYPE_UINT: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                result = LLVMDIBuilderCreateBasicType(data->debug_bulder, name.data, name.length, t->size, 7, 0);
                break;
            }
            case TYPE_REAL: {
                TypeSizedPrimitive* t = (TypeSizedPrimitive*)type;
                result = LLVMDIBuilderCreateBasicType(data->debug_bulder, name.data, name.length, t->size, 5, 0);
                break;
            }
            case TYPE_POINTER: {
                TypePointer* t = (TypePointer*)type;
                LLVMMetadataRef elem = generateLlvmTypeDebugInfo(context, data, t->base, line);
                result = LLVMDIBuilderCreatePointerType(
                    data->debug_bulder, elem,
                    8 * LLVMABISizeOfType(context->target_data, llvm_type),
                    8 * LLVMABIAlignmentOfType(context->target_data, llvm_type), 0, name.data,
                    name.length
                );
                break;
            }
            case TYPE_ARRAY: {
                TypeArray* t = (TypeArray*)type;
                LLVMMetadataRef elem = generateLlvmTypeDebugInfo(context, data, t->base, line);
                LLVMMetadataRef size = LLVMDIBuilderGetOrCreateSubrange(data->debug_bulder, 0, t->size);
                result = LLVMDIBuilderCreateArrayType(
                    data->debug_bulder, 8 * LLVMABIAlignmentOfType(context->target_data, llvm_type),
                    8 * LLVMABIAlignmentOfType(context->target_data, llvm_type), elem, &size, 1
                );
                break;
            }
            case TYPE_FUNCTION: {
                TypeFunction* t = (TypeFunction*)type;
                LLVMMetadataRef* params = ALLOC(LLVMMetadataRef, t->arg_count + 2);
                params[0] = generateLlvmTypeDebugInfo(context, data, t->ret_type, line);
                for (size_t i = 0; i < t->arg_count; i++) {
                    params[i + 1] = generateLlvmTypeDebugInfo(context, data, t->arguments[i], line);
                }
                if (t->vararg) {
                    params[t->arg_count + 1] = NULL;
                }
                result = LLVMDIBuilderCreateSubroutineType(
                    data->debug_bulder, data->file_metadata, params,
                    t->vararg ? t->arg_count + 2 : t->arg_count + 1, 0
                );
                FREE(params);
                break;
            }
            case TYPE_REFERENCE: {
                TypeReference* t = (TypeReference*)type;
                SymbolType* binding = (SymbolType*)t->binding;
                if (binding->codegen != NULL) {
                    result = t->binding->codegen;
                } else {
                    AstTypeDef* n = (AstTypeDef*)type->def;
                    binding->codegen = tmp;
                    if (n != NULL) {
                        result = LLVMDIBuilderCreateTypedef(
                            data->debug_bulder,
                            generateLlvmTypeDebugInfo(context, data, binding->type, line), binding->name,
                            strlen(binding->name), data->file_metadata, n->location.begin.line + 1,
                            data->file_metadata,
                            8 * LLVMABIAlignmentOfType(context->target_data, llvm_type)
                        );
                    } else {
                        result = generateLlvmTypeDebugInfo(context, data, binding->type, line);
                    }
                    binding->codegen = result;
                }
                break;
            }
            case TYPE_UNSURE: {
                TypeUnsure* t = (TypeUnsure*)type;
                if (t->actual != NULL) {
                    result = generateLlvmTypeDebugInfo(context, data, t->actual, line);
                } else {
                    result = generateLlvmTypeDebugInfo(context, data, t->fallback, line);
                }
                break;
            }
        }
        freeString(name);
        LLVMMetadataReplaceAllUsesWith(tmp, result);
        CODEGEN(type)->debug = result;
    }
    return CODEGEN(type)->debug;
}

