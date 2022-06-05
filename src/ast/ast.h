#ifndef _AST_AST_H_
#define _AST_AST_H_

#include <stddef.h>
#include <stdint.h>

#include "files/file.h"
#include "text/string.h"
#include "compiler/symboltable.h"
#include "text/symbol.h"

typedef enum {
    AST_ERROR,
    AST_NEVER,

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
    AST_NOT,
    AST_RETURN,

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
    AST_ROOT,
    AST_BLOCK,
} AstNodeKind;

#define AST_NODE_BASE \
    AstNodeKind kind;       \
    struct AstNode* parent; \
    Span location;          \
    const Type* res_type;

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

typedef struct AstVar {
    AST_NODE_BASE
    Symbol name;
    SymbolEntry* binding;
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
} AstRoot;

typedef struct {
    AST_NODE_BASE
    SymbolTable vars;
    AstList* nodes;
} AstBlock;

AstNode* createAstSimple(Span loc, AstNodeKind kind);

AstBinary* createAstBinary(Span loc, AstNodeKind kind, AstNode* left, AstNode* right);

AstUnary* createAstUnary(Span loc, AstNodeKind kind, AstNode* operand);

AstList* createAstList(Span loc, AstNodeKind kind, size_t count, AstNode** nodes);

AstRoot* createAstRoot(Span loc, AstList* nodes);

AstBlock* createAstBlock(Span loc, AstList* nodes);

AstIfElse* createAstIfElse(Span loc, AstNode* cond, AstNode* if_block, AstNode* else_block);

AstVar* createAstVar(Span loc, Symbol name);

AstVarDef* createAstVarDef(Span loc, AstVar* name, AstNode* type, AstNode* val);

AstWhile* createAstWhile(Span loc, AstNode* cond, AstNode* block);

AstFn* createAstFn(Span loc, AstVar* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags);

AstCall* createAstCall(Span loc, AstNode* func, AstList* arguments);

AstInt* createAstInt(Span loc, AstIntType num);

AstReal* createAstReal(Span loc, AstRealType num);

AstStr* createAstStr(Span loc, String string);

AstTypeDef* createAstTypeDef(Span loc, AstVar* name, AstNode* value);

AstArgDef* createAstArgDef(Span loc, AstVar* name, AstNode* type);

void freeAstNode(AstNode* node);

#endif
