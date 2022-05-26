#ifndef _AST_AST_H_
#define _AST_AST_H_

#include <stddef.h>
#include <stdint.h>

typedef enum {
    AST_NONE,

// AstList
    AST_ROOT,
    AST_BLOCK,

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
    AST_TYPEDEF,
    AST_ARGDEF,
    AST_INDEX,

// AstUnary
    AST_POS,
    AST_NEG,
    AST_ADDR,
    AST_DEREF,

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
    const char* name;
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

typedef struct {
    AST_NODE_BASE
    char* name;
    AstList* arguments;
    AstNode* ret_type;
    AstNode* body;
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

AstBinary* createAstBinary(AstNodeKind kind, AstNode* left, AstNode* right);

AstUnary* createAstUnary(AstNodeKind kind, AstNode* operand);

AstList* createAstList(AstNodeKind kind, size_t count, AstNode** nodes);

AstIfElse* createAstIfElse(AstNode* cond, AstNode* if_block, AstNode* else_block);

AstVar* createAstVar(const char* name);

AstVarDef* createAstVarDef(AstNode* dst, AstNode* type, AstNode* val);

AstWhile* createAstWhile(AstNode* cond, AstNode* block);

AstFn* createAstFn(const char* name, AstList* arguments, AstNode* ret_type, AstNode* body);

AstCall* createAstCall(AstNode* func, AstList* arguments);

AstInt* createAstInt(const char* string);

AstReal* createAstReal(const char* string);

AstStr* createAstStr(const char* string);

void freeAstNode(AstNode* node);

#endif
