
#include "ast/astprinter.h"
#include "ast/ast.h"

static const char* ast_type_names[] = {
    [AST_LIST] = "list",

    // AstList
    [AST_ROOT] = "root",
    [AST_BLOCK] = "block",

    // AstBinary
    [AST_ADD] = "add",
    [AST_SUB] = "sub",
    [AST_MUL] = "mul",
    [AST_DIV] = "div",
    [AST_MOD] = "mod",
    [AST_OR] = "or",
    [AST_AND] = "and",
    [AST_SHL] = "shl",
    [AST_SHR] = "shr",
    [AST_BAND] = "band",
    [AST_BOR] = "bor",
    [AST_BXOR] = "bxor",
    [AST_EQ] = "eq",
    [AST_NE] = "ne",
    [AST_LE] = "le",
    [AST_GE] = "ge",
    [AST_LT] = "lt",
    [AST_GT] = "gt",
    [AST_ASSIGN] = "assign",
    [AST_TYPEDEF] = "typedef",
    [AST_ARGDEF] = "argdef",
    [AST_INDEX] = "index",

    // AstUnary
    [AST_POS] = "pos",
    [AST_NEG] = "neg",
    [AST_ADDR] = "addr",
    [AST_DEREF] = "deref",

    // Other
    [AST_VAR] = "var",
    [AST_VARDEF] = "vardef",
    [AST_IF_ELSE] = "if_else",
    [AST_WHILE] = "while",
    [AST_FN] = "fn",
    [AST_CALL] = "call",
    [AST_INT] = "int",
    [AST_REAL] = "real",
    [AST_STR] = "str",
};

void printAstIndented(FILE* file, AstNode* node, int indent) {
    char indentation[indent + 1];
    for(int i = 0; i < indent; i++) {
        indentation[i] = ' ';
    }
    indentation[indent] = 0;
    if (node == NULL) {
        fprintf(file, "%snull", indentation);
    } else {
        fprintf(file, "%sNode %s\n", indentation, ast_type_names[node->kind]);
        switch (node->kind) {
            case AST_LIST:
            case AST_ROOT:
            case AST_BLOCK: {
                AstList* n = (AstList*)node;
                fprintf(file, "%s .nodes:\n", indentation);
                for (size_t i = 0; i < n->count; i++) {
                    printAstIndented(file, n->nodes[i], indent + 3);
                }
                break;
            }
            case AST_ASSIGN:
            case AST_TYPEDEF:
            case AST_ARGDEF:
            case AST_INDEX:
            case AST_SUB:
            case AST_MUL:
            case AST_DIV:
            case AST_MOD:
            case AST_OR:
            case AST_AND:
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
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                fprintf(file, "%s .left:\n", indentation);
                printAstIndented(file, n->left, indent + 3);
                fprintf(file, "%s .right:\n", indentation);
                printAstIndented(file, n->right, indent + 3);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                fprintf(file, "%s .op:\n", indentation);
                printAstIndented(file, n->op, indent + 3);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                fprintf(file, "%s .name: %s\n", indentation, n->name);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                fprintf(file, "%s .dst:\n", indentation);
                printAstIndented(file, n->dst, indent + 3);
                fprintf(file, "%s .type:\n", indentation);
                printAstIndented(file, n->type, indent + 3);
                fprintf(file, "%s .val:\n", indentation);
                printAstIndented(file, n->val, indent + 3);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                fprintf(file, "%s .condition:\n", indentation);
                printAstIndented(file, n->condition, indent + 3);
                fprintf(file, "%s .ifblock:\n", indentation);
                printAstIndented(file, n->if_block, indent + 3);
                fprintf(file, "%s .elseblock:\n", indentation);
                printAstIndented(file, n->else_block, indent + 3);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                fprintf(file, "%s .condition:\n", indentation);
                printAstIndented(file, n->condition, indent + 3);
                fprintf(file, "%s .block:\n", indentation);
                printAstIndented(file, n->block, indent + 3);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                fprintf(file, "%s .name: %s\n", indentation, n->name);
                fprintf(file, "%s .arguments:\n", indentation);
                printAstIndented(file, (AstNode*)n->arguments, indent + 3);
                fprintf(file, "%s .ret_type:\n", indentation);
                printAstIndented(file, n->ret_type, indent + 3);
                fprintf(file, "%s .body:\n", indentation);
                printAstIndented(file, n->body, indent + 3);
                break;

            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                fprintf(file, "%s .function:\n", indentation);
                printAstIndented(file, n->function, indent + 3);
                fprintf(file, "%s .arguments:\n", indentation);
                printAstIndented(file, (AstNode*)n->arguments, indent + 3);
                break;
            }
            case AST_STR: {
                AstStr* n = (AstStr*)node;
                fprintf(file, "%s .string: %s\n", indentation, n->string);
                break;
            }
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                fprintf(file, "%s .int: %ji\n", indentation, n->number);
                break;
            }
            case AST_REAL: {
                AstReal* n = (AstReal*)node;
                fprintf(file, "%s .real: %lg\n", indentation, n->number);
                break;
            }
        }
    }
}

void printAst(FILE* file, AstNode* ast) {
    printAstIndented(file, ast, 0);
}
