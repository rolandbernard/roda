
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "ast/ast.h"
#include "util/alloc.h"

#include "ast/astprinter.h"

static const char* ast_type_names[] = {
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
    [AST_ARGDEF] = "argdef",
    [AST_INDEX] = "index",
    [AST_ARRAY] = "array",

    // AstUnary
    [AST_POS] = "pos",
    [AST_NEG] = "neg",
    [AST_ADDR] = "addr",
    [AST_DEREF] = "deref",
    [AST_RETURN] = "return",

    // AstList
    [AST_LIST] = "list",
    [AST_ROOT] = "root",
    [AST_BLOCK] = "block",

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
    [AST_TYPEDEF] = "typedef",
};

typedef struct {
    char* data;
    size_t count;
    size_t capacity;
} IndentStack;

void initIndentStack(IndentStack* stack) {
    stack->data = ALLOC(char, 32);
    stack->count = 0;
    stack->capacity = 32;
}

void deinitIndentStack(IndentStack* stack) {
    FREE(stack->data);
}

void pushIndentStack(IndentStack* stack, size_t off, const char* chars) {
    stack->count = off;
    while (*chars != 0) {
        if (stack->count == stack->capacity) {
            stack->capacity *= 2;
            stack->data = REALLOC(char, stack->data, stack->capacity);
        }
        stack->data[stack->count] = *chars;
        stack->count++;
        chars++;
    }
}

static void printAstIndented(FILE* file, AstNode* node, bool colors, IndentStack* indent);

static void printAstChildNode(FILE* file, AstNode* node, bool colors, IndentStack* indent, const char* name, bool last) {
    size_t indentation = indent->count;
    if (name != NULL) {
        fwrite(indent->data, 1, indentation, file);
        if (colors) {
            fprintf(file, " \e[97m│\e[m\e[2;3m%s:\e[m\n", name);
        } else {
            fprintf(file, " │%s:\n", name);
        }
    }
    fwrite(indent->data, 1, indentation, file);
    if (colors) {
        if (last) {
            fprintf(file, " \e[97m└─\e[m");
            pushIndentStack(indent, indentation, "   ");
        } else {
            fprintf(file, " \e[97m├─\e[m");
            pushIndentStack(indent, indentation, " \e[97m│\e[m ");
        }
    } else {
        if (last) {
            fprintf(file, " └─");
            pushIndentStack(indent, indentation, "   ");
        } else {
            fprintf(file, " ├─");
            pushIndentStack(indent, indentation, " │ ");
        }
    }
    printAstIndented(file, node, colors, indent);
    indent->count = indentation;
}

static void printAstIndented(FILE* file, AstNode* node, bool colors, IndentStack* indent) {
    if (colors) {
        fprintf(file, "\e[1;97m");
    }
    if (node == NULL) {
        fprintf(file, "null\n");
    } else {
        fprintf(file, "%s", ast_type_names[node->kind]);
    }
    if (colors) {
        fprintf(file, "\e[m");
    }
    if (node != NULL) {
        switch (node->kind) {
            case AST_ASSIGN:
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
            case AST_ARRAY:
            case AST_ADD: {
                AstBinary* n = (AstBinary*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->left, colors, indent, "left", false);
                printAstChildNode(file, n->right, colors, indent, "right", true);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->op, colors, indent, "op", true);
                break;
            }
            case AST_LIST:
            case AST_ROOT:
            case AST_BLOCK: {
                AstList* n = (AstList*)node;
                fprintf(file, " (%zi elements)\n", n->count);
                for (size_t i = 0; i < n->count; i++) {
                    printAstChildNode(file, n->nodes[i], colors, indent, NULL, i == n->count - 1);
                }
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                fprintf(file, " (name = %s)\n", n->name);
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->dst, colors, indent, "dst", false);
                printAstChildNode(file, n->type, colors, indent, "type", false);
                printAstChildNode(file, n->val, colors, indent, "val", true);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->condition, colors, indent, "condition", false);
                printAstChildNode(file, n->if_block, colors, indent, "if_block", false);
                printAstChildNode(file, n->else_block, colors, indent, "else_block", true);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->condition, colors, indent, "condition", false);
                printAstChildNode(file, n->block, colors, indent, "block", true);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                fprintf(file, " (name = %s, flags =", n->name);
                if (n->flags == AST_FN_FLAG_NONE) {
                    fprintf(file, " none");
                } else {
                    if ((n->flags & AST_FN_FLAG_EXPORT) != 0) {
                        fprintf(file, " export");
                    }
                    if ((n->flags & AST_FN_FLAG_IMPORT) != 0) {
                        fprintf(file, " import");
                    }
                }
                fprintf(file, ")\n");
                printAstChildNode(file, (AstNode*)n->arguments, colors, indent, "arguments", false);
                printAstChildNode(file, n->ret_type, colors, indent, "ret_type", false);
                printAstChildNode(file, n->body, colors, indent, "body", true);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                fprintf(file, "\n");
                printAstChildNode(file, n->function, colors, indent, "function", false);
                printAstChildNode(file, (AstNode*)n->arguments, colors, indent, "arguments", true);
                break;
            }
            case AST_STR: {
                AstStr* n = (AstStr*)node;
                fprintf(file, " (string = %s)\n", n->string);
                break;
            }
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                fprintf(file, " (int = %ji)\n", n->number);
                break;
            }
            case AST_REAL: {
                AstReal* n = (AstReal*)node;
                fprintf(file, " (int = %lg)\n", n->number);
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                fprintf(file, " (name = %s)\n", n->name);
                printAstChildNode(file, n->value, colors, indent, "value", true);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                fprintf(file, " (name = %s)\n", n->name);
                printAstChildNode(file, n->type, colors, indent, "type", true);
                break;
            }
        }
    }
}

void printAst(FILE* file, AstNode* ast) {
    IndentStack stack;
    initIndentStack(&stack);
    printAstIndented(file, ast, isatty(fileno(file)), &stack);
    deinitIndentStack(&stack);
}
