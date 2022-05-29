#ifndef _AST_AST_H_
#define _AST_AST_H_

#include <stddef.h>
#include <stdint.h>

#include "text/string.h"
#include "compiler/symbols.h"

typedef enum {
    AST_ERROR,

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
    AST_ADD_ASSIGN,
    AST_SUB_ASSIGN,
    AST_MUL_ASSIGN,
    AST_DIV_ASSIGN,
    AST_MOD_ASSIGN,
    AST_SHL_ASSIGN,
    AST_SHR_ASSIGN,
    AST_BAND_ASSIGN,
    AST_BOR_ASSIGN,
    AST_BXOR_ASSIGN,

    // AstUnary
    AST_POS,
    AST_NEG,
    AST_ADDR,
    AST_DEREF,
    AST_RETURN,

    // AstBlock
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
    AST_LIST,
} AstNodeKind;

#define AST_NODE_BASE \
    struct AstNode* parent;  \
    AstNodeKind kind;

typedef struct AstNode {
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
    String name;
} AstVar;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
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
    SymbolTable vars;
    AstVar* name;
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

typedef uintmax_t AstIntType;

typedef struct {
    AST_NODE_BASE
    AstIntType number;
} AstInt;

typedef double AstRealType;

typedef struct {
    AST_NODE_BASE
    AstRealType number;
} AstReal;

typedef struct {
    AST_NODE_BASE
    String string;
} AstStr;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
    AstNode* value;
} AstTypeDef;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
    AstNode* type;
} AstArgDef;

typedef struct {
    AST_NODE_BASE
    SymbolTable vars;
    AstList* nodes;
} AstBlock;

AstNode* createAstSimple(AstNodeKind kind);

AstBinary* createAstBinary(AstNodeKind kind, AstNode* left, AstNode* right);

AstUnary* createAstUnary(AstNodeKind kind, AstNode* operand);

AstList* createAstList(AstNodeKind kind, size_t count, AstNode** nodes);

AstBlock* createAstBlock(AstNodeKind kind, AstList* nodes);

AstIfElse* createAstIfElse(AstNode* cond, AstNode* if_block, AstNode* else_block);

AstVar* createAstVar(String name);

AstVarDef* createAstVarDef(AstVar* name, AstNode* type, AstNode* val);

AstWhile* createAstWhile(AstNode* cond, AstNode* block);

AstFn* createAstFn(AstVar* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags);

AstCall* createAstCall(AstNode* func, AstList* arguments);

AstInt* createAstInt(AstIntType num);

AstReal* createAstReal(AstRealType num);

AstStr* createAstStr(String string);

AstTypeDef* createAstTypeDef(AstVar* name, AstNode* value);

AstArgDef* createAstArgDef(AstVar* name, AstNode* type);

void freeAstNode(AstNode* node);

#endif
