#ifndef _RODA_AST_AST_H_
#define _RODA_AST_AST_H_

#include <stddef.h>
#include <stdint.h>

#include "files/file.h"
#include "text/string.h"
#include "compiler/symboltable.h"
#include "text/symbol.h"

typedef enum {
    AST_ERROR,
    AST_VOID,

    // AstBinary
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_MOD,
    AST_SHL,
    AST_SHR,
    AST_BAND,
    AST_BOR,
    AST_BXOR,
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
#define AST_ASSIGN_OFFSET (AST_ADD_ASSIGN - AST_ADD)

    AST_OR,
    AST_AND,
    AST_EQ,
    AST_NE,
    AST_LE,
    AST_GE,
    AST_LT,
    AST_GT,
    AST_ASSIGN,
    AST_INDEX,
    AST_ARRAY,
    AST_AS,

    // AstUnary
    AST_POS,
    AST_NEG,
    AST_ADDR,
    AST_DEREF,
    AST_NOT,
    AST_SIZEOF,

    // Other
    AST_VAR,
    AST_VARDEF,
    AST_CONSTDEF,
    AST_STATICDEF,
    AST_IF_ELSE,
    AST_IF_ELSE_EXPR,
    AST_WHILE,
    AST_FN,
    AST_CALL,
    AST_TYPEDEF,
    AST_ARGDEF,
    AST_FIELD_DEF = AST_ARGDEF,
    AST_FIELD_VAL = AST_ARGDEF,
    AST_LIST,
    AST_ROOT,
    AST_BLOCK,
    AST_BLOCK_EXPR,
    AST_RETURN,
    AST_ARRAY_LIT,
    AST_FN_TYPE,
    AST_STRUCT_TYPE,
    AST_STRUCT_INDEX,
    AST_STRUCT_LIT,
    AST_TUPLE_TYPE,
    AST_TUPLE_INDEX,
    AST_TUPLE_LIT,
    AST_BREAK,
    AST_CONTINUE,

    AST_INT,
    AST_CHAR,
    AST_REAL,
    AST_STR,
    AST_BOOL,
} AstNodeKind;

#define AST_NODE_BASE                   \
    AstNodeKind kind;                   \
    struct AstNode* parent;             \
    size_t parent_idx;                  \
    Span location;                      \
    Type* res_type;                     \
    struct AstNode* type_ref_next;      \
    void* codegen;

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
    size_t capacity;
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
    struct AstVar* next_ref;
} AstVar;

typedef enum {
    AST_VAR_FLAG_NONE   = 0,
    AST_VAR_FLAG_PUBLIC = 1 << 0,
    AST_VAR_FLAG_EXTERN = 1 << 1,
} AstVarFlags;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
    AstNode* type;
    AstNode* val;
    AstVarFlags flags;
} AstVarDef;

typedef struct {
    AST_NODE_BASE
    AstNode* condition;
    AstNode* block;
} AstWhile;

typedef enum {
    AST_FN_FLAG_NONE   = 0,
    AST_FN_FLAG_PUBLIC = 1 << 0,
    AST_FN_FLAG_EXTERN = 1 << 1,
    AST_FN_FLAG_VARARG = 1 << 2,
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

typedef BigInt* AstIntType;

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
    bool value;
} AstBool;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
    AstNode* value;
} AstTypeDef;

typedef struct {
    AST_NODE_BASE
    AstVar* name;
    AstNode* type;
#define field_value type
} AstArgDef;

typedef AstArgDef AstStructField;

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

typedef struct {
    AST_NODE_BASE
    AstNode* value;
    AstFn* function;
} AstReturn;

typedef struct {
    AST_NODE_BASE
    AstList* arguments;
    AstNode* ret_type;
    bool vararg;
} AstFnType;

typedef struct {
    AST_NODE_BASE
    AstNode* strct;
    AstVar* field;
} AstStructIndex;

typedef struct {
    AST_NODE_BASE
    AstNode* tuple;
    AstInt* field;
} AstTupleIndex;

typedef struct {
    AST_NODE_BASE
    AstNode* break_target;
} AstBreak;

AstNode* createAstSimple(Span loc, AstNodeKind kind);

AstBinary* createAstBinary(Span loc, AstNodeKind kind, AstNode* left, AstNode* right);

AstUnary* createAstUnary(Span loc, AstNodeKind kind, AstNode* operand);

AstList* createAstList(Span loc, AstNodeKind kind, size_t count, AstNode** nodes);

AstRoot* createAstRoot(Span loc, AstList* nodes);

AstBlock* createAstBlock(Span loc, AstNodeKind kind, AstList* nodes);

AstIfElse* createAstIfElse(Span loc, AstNodeKind kind, AstNode* cond, AstNode* if_block, AstNode* else_block);

AstVar* createAstVar(Span loc, Symbol name);

AstVarDef* createAstVarDef(Span loc, AstNodeKind kind, AstVar* name, AstNode* type, AstNode* val, AstVarFlags flags);

AstWhile* createAstWhile(Span loc, AstNode* cond, AstNode* block);

AstFn* createAstFn(Span loc, AstVar* name, AstList* arguments, AstNode* ret_type, AstNode* body, AstFnFlags flags);

AstCall* createAstCall(Span loc, AstNode* func, AstList* arguments);

AstInt* createAstInt(Span loc, AstNodeKind kind, AstIntType num);

AstReal* createAstReal(Span loc, AstRealType num);

AstStr* createAstStr(Span loc, String string);

AstBool* createAstBool(Span loc, bool value);

AstTypeDef* createAstTypeDef(Span loc, AstVar* name, AstNode* value);

AstArgDef* createAstArgDef(Span loc, AstVar* name, AstNode* type);

AstReturn* createAstReturn(Span loc, AstNode* value);

AstFnType* createAstFnType(Span loc, AstList* arguments, AstNode* ret_type, bool vararg);

AstStructIndex* createAstStructIndex(Span loc, AstNode* tuple, AstVar* field);

AstTupleIndex* createAstTupleIndex(Span loc, AstNode* strct, AstInt* field);

AstBreak* createAstBreak(Span loc, AstNodeKind kind);

void freeAstNode(AstNode* node);

void initAstNode(AstNode* node, AstNodeKind kind, Span loc);

#endif
