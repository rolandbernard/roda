
#include <string.h>

#include "compiler/consteval.h"
#include "compiler/typecheck.h"
#include "compiler/typeinfer.h"
#include "errors/fatalerror.h"
#include "errors/msgkind.h"
#include "text/format.h"
#include "util/alloc.h"
#include "util/sort.h"

#include "compiler/typeeval.h"

static void swapStructFields(size_t i, size_t j, void* cxt) {
    AstStructField** fields = (AstStructField**)cxt;
    swap(fields, sizeof(AstStructField*), i, j);
}

static bool compareStructFieldNames(size_t i, size_t j, void* cxt) {
    AstStructField** fields = (AstStructField**)cxt;
    if (fields[i]->name->name != fields[j]->name->name) {
        return fields[i]->name->name <= fields[j]->name->name;
    } else {
        return fields[i]->location.begin.offset <= fields[j]->location.begin.offset;
    }
}

void sortStructFieldsByName(AstList* n) {
    heapSort(n->count, swapStructFields, compareStructFieldNames, n->nodes);
}

bool checkStructFieldsHaveNoDups(CompilerContext* context, AstList* n) {
    sortStructFieldsByName(n);
    bool error = false;
    for (size_t i = 1; i < n->count; i++) {
        AstStructField* last = (AstStructField*)n->nodes[i - 1];
        AstStructField* field = (AstStructField*)n->nodes[i];
        if (field->name->name == last->name->name) {
            addMessageToContext(
                &context->msgs,
                createMessage(
                    ERROR_DUPLICATE_STRUCT_FIELD,
                    createFormattedString(
                        "duplicate definition of struct field named `%s`", field->name->name
                    ),
                    2,
                    createMessageFragment(
                        MESSAGE_ERROR, copyFromCString("duplicate field definition"),
                        field->name->location
                    ),
                    createMessageFragment(
                        MESSAGE_NOTE, copyFromCString("note: previously defined here"),
                        last->name->location
                    )
                )
            );
            error = true;
            break;
        }
    }
    return error;
}

Type* evaluateTypeExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE("should not evaluate");
    } else {
        switch (node->kind) {
            case AST_VOID: {
                node->res_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                break;
            }
            case AST_ERROR: {
                node->res_type = getErrorType(&context->types);
                break;
            }
            case AST_IF_ELSE:
            case AST_FN:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_WHILE:
            case AST_ADD_ASSIGN:
            case AST_SUB_ASSIGN:
            case AST_MUL_ASSIGN:
            case AST_DIV_ASSIGN:
            case AST_MOD_ASSIGN:
            case AST_SHL_ASSIGN:
            case AST_SHR_ASSIGN:
            case AST_BAND_ASSIGN:
            case AST_BOR_ASSIGN:
            case AST_BXOR_ASSIGN:
            case AST_ASSIGN:
            case AST_RETURN:
            case AST_INDEX:
            case AST_STRUCT_INDEX:
            case AST_AS:
            case AST_CALL:
            case AST_STR:
            case AST_DEREF:
            case AST_INT:
            case AST_CHAR:
            case AST_BOOL:
            case AST_REAL:
            case AST_ADD:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_SHL:
            case AST_SHR:
            case AST_BAND:
            case AST_BOR:
            case AST_BXOR:
            case AST_EQ:
            case AST_NE:
            case AST_LE:
            case AST_GE:
            case AST_LT:
            case AST_GT:
            case AST_OR:
            case AST_AND:
            case AST_POS:
            case AST_NEG:
            case AST_NOT:
            case AST_SIZEOF:
            case AST_ROOT:
            case AST_ARRAY_LIT:
            case AST_STRUCT_LIT:
            case AST_LIST:
            case AST_BLOCK:
            case AST_BLOCK_EXPR:
            case AST_VARDEF: {
                UNREACHABLE("should not evaluate");
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                if (n->binding == NULL) {
                    n->res_type = getErrorType(&context->types);
                } else {
                    SymbolType* var = (SymbolType*)n->binding;
                    n->res_type = createTypeReference(&context->types, node, var);
                    if (var->type == NULL) {
                        AstTypeDef* def = (AstTypeDef*)var->def->parent;
                        var->type = getErrorType(&context->types);
                        var->type = evaluateTypeExpr(context, def->value);
                    }
                }
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                Type* base = evaluateTypeExpr(context, n->op);
                n->res_type = createPointerType(&context->types, node, base);
                break;
            }
            case AST_ARRAY: {
                AstBinary* n = (AstBinary*)node;
                Type* base = evaluateTypeExpr(context, n->right);
                if (checkValidInConstExpr(context, n->left)) {
                    typeInferExpr(context, n->left, createSizedPrimitiveType(&context->types, node, TYPE_UINT, SIZE_SIZE));
                    typeCheckExpr(context, n->left);
                }
                if (context->msgs.error_count != 0) {
                    n->res_type = getErrorType(&context->types);
                } else {
                    ConstValue size = evaluateConstExpr(context, n->left);
                    if (isErrorType(size.type)) {
                        n->res_type = size.type;
                    } else if (!isIntegerType(size.type)) {
                        String idx_type = buildTypeName(size.type);
                        addMessageToContext(
                            &context->msgs,
                            createMessage(
                                ERROR_INCOMPATIBLE_TYPE,
                                createFormattedString("array length with non integer type `%s`", cstr(idx_type)), 1,
                                createMessageFragment(MESSAGE_ERROR, createFormattedString("type `%s` is not an integer type", cstr(idx_type)), n->left->location)
                            )
                        );
                        freeString(idx_type);
                        n->res_type = getErrorType(&context->types);
                    } else if (isSignedIntegerType(size.type) && size.sint < 0) {
                        addMessageToContext(
                            &context->msgs,
                            createMessage(
                                ERROR_INVALID_ARRAY_LENGTH,
                                createFormattedString("negative array length, `%i` is less than 0", size.sint), 1,
                                createMessageFragment(MESSAGE_ERROR, createFormattedString("array length of `%i` not allowed here", size.sint), n->left->location)
                            )
                        );
                        n->res_type = getErrorType(&context->types);
                    } else {
                        size_t len = isSignedIntegerType(size.type) ? (size_t)size.sint : (size_t)size.uint;
                        n->res_type = createArrayType(&context->types, node, base, len);
                    }
                }
                break;
            }
            case AST_FN_TYPE: {
                AstFnType* n = (AstFnType*)node;
                Type** args = ALLOC(Type*, n->arguments->count);
                for (size_t i = 0; i < n->arguments->count; i++) {
                    args[i] = evaluateTypeExpr(context, n->arguments->nodes[i]);
                }
                Type* ret_type = NULL;
                if (n->ret_type != NULL) {
                    ret_type = evaluateTypeExpr(context, n->ret_type);
                } else {
                    ret_type = createUnsizedPrimitiveType(&context->types, node, TYPE_VOID);
                }
                n->res_type = createFunctionType(&context->types, node, ret_type, n->arguments->count, args, n->vararg);
                break;
            }
            case AST_STRUCT_TYPE: {
                AstList* n = (AstList*)node;
                if (!checkStructFieldsHaveNoDups(context, n)) {
                    Symbol* names = ALLOC(Symbol, n->count);
                    Type** types = ALLOC(Type*, n->count);
                    for (size_t i = 0; i < n->count; i++) {
                        AstStructField* field = (AstStructField*)n->nodes[i];
                        names[i] = field->name->name;
                        types[i] = evaluateTypeExpr(context, field->type);
                    }
                    n->res_type = createTypeStruct(&context->types, node, false, names, types, n->count);
                } else {
                    n->res_type = getErrorType(&context->types);
                }
                break;
            }
        }
        return node->res_type;
    }
}

