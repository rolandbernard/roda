#ifndef _AST_AST_H_
#define _AST_AST_H_

#include <stddef.h>
#include <stdint.h>

typedef enum {
    // AstBinary
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_OR,
    AST_AND,
    AST_SHL,
    AST_SHR,
    AST_BAND,
    AST_BOR,
    AST_BXOR,
    AST_EQ,
    AST_NE,
    AST_LE,
    AST_GE,
    AST_LT,
    AST_GT,
    AST_ASSIGN,
    AST_INDEX,
    AST_ARRAY,

    // AstUnary
    AST_POS,
    AST_NEG,
    AST_ADDR,
    AST_DEREF,
    AST_RETURN,

    // AstList
    AST_LIST,
    AST_ROOT,
    AST_BLOCK,

    // Other
    AST_VAR,
    AST_VARDEF,
    AST_IF_ELSE,
    AST_WHILE,
    AST_FN,
    AST_CALL,
    AST_INT,
    AST_REAL,
    AST_STR,
    AST_TYPEDEF,
    AST_ARGDEF,
} AstNodeKind;

#define AST_NODE_BASE \
    AstNodeKind kind;

typedef struct {
    AST_NODE_BASE
} AstNode;

typedef struct {
    AST_NODE_BASE
    AstNode* left;
    AstNode* right;
} AstBinary;

typedef struct {
    AST_NODE_BASE
    AstNode* op;
} AstUnary;

typedef struct {
    AST_NODE_BASE
    AstNode** nodes;
    size_t count;
} AstList;

typedef struct {
    AST_NODE_BASE
    AstNode* condition;
    AstNode* if_block;
    AstNode* else_block;
} AstIfElse;

typedef struct {
    AST_NODE_BASE
    char* name;
} AstVar;

typedef struct {
    AST_NODE_BASE
    AstNode* dst;
    AstNode* type;
    AstNode* val;
} AstVarDef;

typedef struct {
    AST_NODE_BASE
    AstNode* condition;
    AstNode* block;
} AstWhile;

typedef enum {
    AST_FN_FLAG_NONE   = 0,
    AST_FN_FLAG_EXPORT = 1 << 0,
    AST_FN_FLAG_IMPORT = 1 << 1,
} AstFnFlags;

typedef struct {
    AST_NODE_BASE
    char* name;
    AstList* arguments;
    AstNode* ret_type;
    AstNode* body;
    AstFnFlags flags;
} AstFn;

typedef struct {
    AST_NODE_BASE
    AstNode* function;
    AstList* arguments;
} AstCall;

typedef struct {
    AST_NODE_BASE
    uintmax_t number;
} AstInt;

typedef struct {
    AST_NODE_BASE
    double number;
} AstReal;

typedef struct {
    AST_NODE_BASE
    char* string;
} AstStr;

typedef struct {
    AST_NODE_BASE
    char* name;
    AstNode* value;
} AstTypeDef;

typedef struct {
    AST_NODE_BASE
    char* name;
    AstNode* arg_type;
} AstArgDef;

AstBinary* createAstBinary(AstNodeKind kind, AstNode* left, AstNode* right);

AstUnary* createAstUnary(AstNodeKind kind, AstNode* operand);

AstList* createAstList(AstNodeKind kind, size_t count, AstNode** nodes);

AstIfElse* createAstIfElse(AstNode* cond, AstNode* if_block, AstNode* else_block);

AstVar* createAstVar(char* name);

AstVarDef* createAstVarDef(AstNode* dst, AstNode* type, AstNode* val);

AstWhile* createAstWhile(AstNode* cond, AstNode* block);

AstFn* createAstFn(char* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags);

AstCall* createAstCall(AstNode* func, AstList* arguments);

AstInt* createAstInt(char* string);

AstReal* createAstReal(char* string);

AstStr* createAstStr(char* string);

AstTypeDef* createAstTypeDef(char* name, AstNode* value);

AstArgDef* createAstArgDef(char* name, AstNode* type);

void freeAstNode(AstNode* node);

#endif