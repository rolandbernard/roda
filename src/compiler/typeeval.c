
#include "compiler/consteval.h"
#include "errors/fatalerror.h"
#include "errors/msgkind.h"
#include "text/format.h"

#include "compiler/typeeval.h"

Type* evaluateTypeExpr(CompilerContext* context, AstNode* node) {
    if (node == NULL) {
        UNREACHABLE(", should not evaluate");
    } else {
        switch (node->kind) {
            case AST_VOID: {
                node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_VOID);
                break;
            }
            case AST_ERROR: {
                node->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
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
            case AST_CALL:
            case AST_STR:
            case AST_DEREF:
            case AST_INT:
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
            case AST_ROOT:
            case AST_LIST:
            case AST_BLOCK:
            case AST_VARDEF: {
                UNREACHABLE(", should not evaluate");
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                if (n->binding == NULL) {
                    n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                } else {
                    SymbolVariable* var = (SymbolVariable*)n->binding;
                    n->res_type = createTypeReference(&context->types, (SymbolType*)var);;
                }
                break;
            }
            case AST_ADDR: {
                AstUnary* n = (AstUnary*)node;
                Type* base = evaluateTypeExpr(context, n->op);
                if (base->kind == TYPE_ERROR) {
                    n->res_type = base;
                } else {
                    n->res_type = createPointerType(&context->types, base);
                }
                break;
            }
            case AST_ARRAY: {
                AstBinary* n = (AstBinary*)node;
                Type* base = evaluateTypeExpr(context, n->right);
                ConstValue size = evaluateConstExpr(context, n->left);
                TypeSizedPrimitive* size_type = isIntegerType(size.type);
                if (base->kind == TYPE_ERROR) {
                    n->res_type = base;
                } else if (size.type->kind == TYPE_ERROR) {
                    n->res_type = size.type;
                } else if (size_type == NULL) {
                    String idx_type = buildTypeName(size.type);
                    addMessageToContext(
                        &context->msgs,
                        createMessage(
                            ERROR_INCOMPATIBLE_TYPE,
                            createFormattedString("array length with non integer type `%S`", idx_type), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("type `%S` is not an integer type", idx_type), n->left->location)
                        )
                    );
                    freeString(idx_type);
                    n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                } else if (size_type->kind == TYPE_INT && size.sint < 0) {
                    addMessageToContext(
                        &context->msgs,
                        createMessage(
                            ERROR_INVALID_ARRAY_LENGTH,
                            createFormattedString("negative array length, `%i` is less than 0", size.sint), 1,
                            createMessageFragment(MESSAGE_ERROR, createFormattedString("array length of `%i` not allowed here", size.sint), n->left->location)
                        )
                    );
                    n->res_type = createUnsizedPrimitiveType(&context->types, TYPE_ERROR);
                } else {
                    size_t len = size_type->kind == TYPE_INT ? (size_t)size.sint : (size_t)size.uint;
                    n->res_type = createArrayType(&context->types, base, len);
                }
                break;
            }
        }
        return node->res_type;
    }
}

