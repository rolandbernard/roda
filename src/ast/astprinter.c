
#include <stdbool.h>
#include <stdint.h>

#include "ast/ast.h"
#include "util/alloc.h"
#include "util/console.h"

#include "ast/astprinter.h"

static const char* ast_type_names[] = {
    [AST_ERROR] = "error",

    // AstBinary
    [AST_ADD] = "add",
    [AST_SUB] = "subtract",
    [AST_MUL] = "multiply",
    [AST_DIV] = "divide",
    [AST_MOD] = "modulo",
    [AST_OR] = "or",
    [AST_AND] = "and",
    [AST_SHL] = "shift left",
    [AST_SHR] = "shift right",
    [AST_BAND] = "binary and",
    [AST_BOR] = "binary or",
    [AST_BXOR] = "binary xor",
    [AST_EQ] = "equal",
    [AST_NE] = "not equal",
    [AST_LE] = "less or equal",
    [AST_GE] = "greater or equal",
    [AST_LT] = "less than",
    [AST_GT] = "greater than",
    [AST_ASSIGN] = "assign",
    [AST_ARGDEF] = "argument definition",
    [AST_INDEX] = "index",
    [AST_ARRAY] = "array",
    [AST_ADD_ASSIGN] = "add assign",
    [AST_SUB_ASSIGN] = "sub assign",
    [AST_MUL_ASSIGN] = "mul assign",
    [AST_DIV_ASSIGN] = "div assign",
    [AST_MOD_ASSIGN] = "mod assign",
    [AST_SHL_ASSIGN] = "shift left assign",
    [AST_SHR_ASSIGN] = "shift right assign",
    [AST_BAND_ASSIGN] = "binary and assign",
    [AST_BOR_ASSIGN] = "binary or assign",
    [AST_BXOR_ASSIGN] = "binary xor assign",

    // AstUnary
    [AST_POS] = "positive",
    [AST_NEG] = "negative",
    [AST_ADDR] = "address of",
    [AST_DEREF] = "dereference",
    [AST_RETURN] = "return",
    [AST_NOT] = "not",

    // AstList
    [AST_LIST] = "list",
    [AST_ROOT] = "root",
    [AST_BLOCK] = "block",

    // Other
    [AST_VAR] = "variable",
    [AST_VARDEF] = "variable definition",
    [AST_IF_ELSE] = "if-else",
    [AST_WHILE] = "while",
    [AST_FN] = "function",
    [AST_CALL] = "call",
    [AST_INT] = "integer",
    [AST_REAL] = "real",
    [AST_STR] = "string",
    [AST_TYPEDEF] = "type definition",
};


const char* getAstPrintName(AstNodeKind kind) {
    return ast_type_names[kind];
}

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

static void printAstChildNode(
    FILE* file, AstNode* node, bool colors, IndentStack* indent, const char* name, bool last
) {
    size_t indentation = indent->count;
    if (name != NULL) {
        fwrite(indent->data, 1, indentation, file);
        if (colors) {
            fprintf(file,
                " " CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_WHITE)
                "│" CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_BLACK;CONSOLE_SGR_ITALIC;CONSOLE_SGR_FAINT)
                "%s:" CONSOLE_SGR() "\n", name
            );
        } else {
            fprintf(file, " │%s:\n", name);
        }
    }
    fwrite(indent->data, 1, indentation, file);
    if (colors) {
        if (last) {
            fprintf(file, CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_WHITE) " └─" CONSOLE_SGR());
            pushIndentStack(indent, indentation, "   ");
        } else {
            fprintf(file, CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_WHITE) " ├─" CONSOLE_SGR());
            pushIndentStack(indent, indentation, CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_WHITE) " │ " CONSOLE_SGR());
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

static void printAstNodeType(FILE* file, AstNode* node, bool colors) {
    if (node->res_type != NULL) {
        if (colors) {
            fprintf(file, CONSOLE_SGR(CONSOLE_SGR_FAINT));
        }
        fprintf(file, " ");
        String type = buildTypeName(node->res_type);
        fwrite(type.data, 1, type.length, file);
        freeString(type);
        if (colors) {
            fprintf(file, CONSOLE_SGR());
        }
    }
}

static void printAstNodeLocation(FILE* file, AstNode* node, bool colors) {
    if (isSpanValid(node->location)) {
        if (colors) {
            fprintf(file, CONSOLE_SGR(CONSOLE_SGR_FG_BRIGHT_BLACK;CONSOLE_SGR_ITALIC));
        }
        fprintf(file, " [");
        if (node->location.file != NULL) {
            fwrite(node->location.file->original_path.data, 1, node->location.file->original_path.length, file);
            fputc(':', file);
        }
        if (node->location.begin.offset != NO_POS) {
            fprintf(file, "%zi:%zi", node->location.begin.line + 1, node->location.begin.column + 1);
            if (node->location.begin.offset + 1 < node->location.end.offset) {
                fprintf(file, "-%zi:%zi", node->location.end.line + 1, node->location.end.column);
            }
        }
        fprintf(file, "]");
        if (colors) {
            fprintf(file, CONSOLE_SGR());
        }
    }
}

static void printAstNodeExtraInfo(FILE* file, AstNode* node, bool colors) {
    printAstNodeType(file, node, colors);
    printAstNodeLocation(file, node, colors);
}

static void printAstIndented(FILE* file, AstNode* node, bool colors, IndentStack* indent) {
    if (colors) {
        fprintf(file, CONSOLE_SGR(CONSOLE_SGR_BOLD;CONSOLE_SGR_FG_BRIGHT_WHITE));
    }
    if (node == NULL) {
        fprintf(file, "null\n");
    } else {
        fprintf(file, "%s", getAstPrintName(node->kind));
    }
    if (colors) {
        fprintf(file, CONSOLE_SGR());
    }
    if (node != NULL) {
        switch (node->kind) {
            case AST_ERROR: {
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                break;
            }
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
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, n->left, colors, indent, "left", false);
                printAstChildNode(file, n->right, colors, indent, "right", true);
                break;
            }
            case AST_POS:
            case AST_NEG:
            case AST_ADDR:
            case AST_RETURN:
            case AST_NOT:
            case AST_DEREF: {
                AstUnary* n = (AstUnary*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, n->op, colors, indent, "op", true);
                break;
            }
            case AST_LIST: {
                AstList* n = (AstList*)node;
                fprintf(file, " (%zi elements)", n->count);
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                for (size_t i = 0; i < n->count; i++) {
                    printAstChildNode(file, n->nodes[i], colors, indent, NULL, i == n->count - 1);
                }
                break;
            }
            case AST_ROOT: {
                AstRoot* n = (AstRoot*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->nodes, colors, indent, "nodes", true);
                break;
            }
            case AST_BLOCK: {
                AstBlock* n = (AstBlock*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->nodes, colors, indent, "nodes", true);
                break;
            }
            case AST_VAR: {
                AstVar* n = (AstVar*)node;
                fprintf(file, " (name = %s)", n->name);
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                break;
            }
            case AST_VARDEF: {
                AstVarDef* n = (AstVarDef*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->name, colors, indent, "name", false);
                printAstChildNode(file, n->type, colors, indent, "type", false);
                printAstChildNode(file, n->val, colors, indent, "val", true);
                break;
            }
            case AST_IF_ELSE: {
                AstIfElse* n = (AstIfElse*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, n->condition, colors, indent, "condition", false);
                printAstChildNode(file, n->if_block, colors, indent, "if_block", false);
                printAstChildNode(file, n->else_block, colors, indent, "else_block", true);
                break;
            }
            case AST_WHILE: {
                AstWhile* n = (AstWhile*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, n->condition, colors, indent, "condition", false);
                printAstChildNode(file, n->block, colors, indent, "block", true);
                break;
            }
            case AST_FN: {
                AstFn* n = (AstFn*)node;
                fprintf(file, " (flags =");
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
                fprintf(file, ")");
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->name, colors, indent, "name", false);
                printAstChildNode(file, (AstNode*)n->arguments, colors, indent, "arguments", false);
                printAstChildNode(file, n->ret_type, colors, indent, "ret_type", false);
                printAstChildNode(file, n->body, colors, indent, "body", true);
                break;
            }
            case AST_CALL: {
                AstCall* n = (AstCall*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, n->function, colors, indent, "function", false);
                printAstChildNode(file, (AstNode*)n->arguments, colors, indent, "arguments", true);
                break;
            }
            case AST_STR: {
                AstStr* n = (AstStr*)node;
                fprintf(file, " (string = %s)", cstr(n->string));
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                break;
            }
            case AST_INT: {
                AstInt* n = (AstInt*)node;
                fprintf(file, " (int = %ji)", n->number);
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                break;
            }
            case AST_REAL: {
                AstReal* n = (AstReal*)node;
                fprintf(file, " (real = %.15lg)", n->number);
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                break;
            }
            case AST_TYPEDEF: {
                AstTypeDef* n = (AstTypeDef*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->name, colors, indent, "name", false);
                printAstChildNode(file, n->value, colors, indent, "value", true);
                break;
            }
            case AST_ARGDEF: {
                AstArgDef* n = (AstArgDef*)node;
                printAstNodeExtraInfo(file, node, colors);
                fprintf(file, "\n");
                printAstChildNode(file, (AstNode*)n->name, colors, indent, "name", false);
                printAstChildNode(file, n->type, colors, indent, "type", true);
                break;
            }
        }
    }
}

void printAst(FILE* file, AstNode* ast) {
    IndentStack stack;
    initIndentStack(&stack);
    printAstIndented(file, ast, isATerminal(file), &stack);
    deinitIndentStack(&stack);
}
