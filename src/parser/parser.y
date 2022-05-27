
%code requires {
#include <stdio.h>

#include "ast/ast.h"
#include "ast/astlist.h"
#include "util/alloc.h"
#include "text/string.h"
#include "errors/msgcontext.h"

extern void yyerror(AstNode**, MessageContext*, const char*);
extern int yylex();
}

%define parse.error detailed

%parse-param {AstNode** ast_result} {MessageContext* context}

%union {
    String lexeme;
    AstNode* ast;
    AstList* list;
    DynamicAstList* dynlist;
}

%destructor { freeString($$); } <lexeme>
%destructor { freeAstNode($$); } <ast>
%destructor { freeAstNode((AstNode*)$$); } <list>
%destructor { freeDynamicAstList($$); } <dynlist>

%type <ast> root block stmt type expr root_stmt arg_def opt_type assign
%type <list> args args_defs
%type <dynlist> args_list args_def_list stmts root_stmts

%token <lexeme> ID      "identifier"
%token <lexeme> STR     "string"
%token <lexeme> INT     "integer"
%token <lexeme> REAL    "real"
%token IF               "if"
%token ELSE             "else"
%token FOR              "for"
%token WHILE            "while"
%token MATCH            "match"
%token EXPORT           "export"
%token IMPORT           "import"
%token CONST            "const"
%token TYPE             "type"
%token LET              "let"
%token FN               "fn"
%token RETURN           "return"
%token EQ               "=="
%token NE               "!="
%token LE               "<="
%token GE               ">="
%token ARROW            "=>"
%token ADD_EQ           "+="
%token SUB_EQ           "-="
%token MUL_EQ           "*="
%token DIV_EQ           "/="
%token MOD_EQ           "%="
%token SHR_EQ           ">>="
%token SHL_EQ           "<<="
%token BOR_EQ           "|="
%token BAND_EQ          "&="
%token BXOR_EQ          "^="
%token OR               "||"
%token AND              "&&"
%token SHR              ">>"
%token SHL              "<<"

%left "||"
%left "&&"
%left '<' '>' "==" "!=" "<=" ">="
%left '|'
%left '^'
%left '&'
%left "<<" ">>"
%left '-' '+'
%left '*' '/' '%'
%precedence UNARY_PRE
%precedence '(' '['

%start program

%%

//todo:
// chars
// for loop
// match
// top level definitions
// 

program : root  { *ast_result = $1; }
        ;

root : root_stmts  { $$ = (AstNode*)createAstBlock(AST_ROOT, toStaticAstList($1)); }
     ;

root_stmts : %empty                     { $$ = createDynamicAstList(); }
           | root_stmts root_stmt       { $$ = $1; addToDynamicAstList($1, $2); }
           ;

root_stmt : "export" "fn" ID '(' args_defs ')' opt_type block   { $$ = (AstNode*)createAstFn($3, $5, $7, $8, AST_FN_FLAG_EXPORT); }
          | "import" "fn" ID '(' args_defs ')' opt_type ';'     { $$ = (AstNode*)createAstFn($3, $5, $7, NULL, AST_FN_FLAG_IMPORT); }
          | "fn" ID '(' args_defs ')' opt_type block          { $$ = (AstNode*)createAstFn($2, $4, $6, $7, AST_FN_FLAG_NONE); }
          ;

opt_type : %empty   { $$ = NULL; }
         | ':' type { $$ = $2; }
         ;

args_defs : %empty              { $$ = createAstList(AST_LIST, 0, NULL); }
          | args_def_list       { $$ = toStaticAstList($1); }
          ;

args_def_list : arg_def                         { $$ = createDynamicAstList(); addToDynamicAstList($$, $1); }
              | args_def_list ',' arg_def       { $$ = $1; addToDynamicAstList($1, $3); }
              ;

arg_def : ID ':' type { $$ = (AstNode*)createAstArgDef($1, $3); }
        ;

stmt    : expr ';'                       { $$ = $1; }
        | assign ';'                     { $$ = $1; }
        | block                          { $$ = $1; }
        | "return" ';'                     { $$ = (AstNode*)createAstUnary(AST_RETURN, NULL); }
        | "return" expr ';'                { $$ = (AstNode*)createAstUnary(AST_RETURN, $2); }
        | "let" ID opt_type '=' expr ';'   { $$ = (AstNode*)createAstVarDef($2, $3, $5); }
        | "let" ID opt_type ';'            { $$ = (AstNode*)createAstVarDef($2, $3, NULL); }
        | "type" ID '=' type ';'           { $$ = (AstNode*)createAstTypeDef($2, $4); }
        | "while" expr block               { $$ = (AstNode*)createAstWhile($2, $3); }
        | "if" expr block                  { $$ = (AstNode*)createAstIfElse($2, $3, NULL); }
        | "if" expr block "else" block       { $$ = (AstNode*)createAstIfElse($2, $3, $5); }
        ;

assign : expr '=' expr          { $$ = (AstNode*)createAstBinary(AST_ASSIGN, $1, $3); }
       | expr "+=" expr       { $$ = (AstNode*)createAstBinary(AST_ADD_ASSIGN, $1, $3); }
       | expr "-=" expr       { $$ = (AstNode*)createAstBinary(AST_SUB_ASSIGN, $1, $3); }
       | expr "*=" expr       { $$ = (AstNode*)createAstBinary(AST_MUL_ASSIGN, $1, $3); }
       | expr "/=" expr       { $$ = (AstNode*)createAstBinary(AST_DIV_ASSIGN, $1, $3); }
       | expr "%=" expr       { $$ = (AstNode*)createAstBinary(AST_MOD_ASSIGN, $1, $3); }
       | expr ">>=" expr       { $$ = (AstNode*)createAstBinary(AST_SHR_ASSIGN, $1, $3); }
       | expr "<<=" expr       { $$ = (AstNode*)createAstBinary(AST_SHL_ASSIGN, $1, $3); }
       | expr "|=" expr       { $$ = (AstNode*)createAstBinary(AST_BOR_ASSIGN, $1, $3); }
       | expr "&=" expr      { $$ = (AstNode*)createAstBinary(AST_BAND_ASSIGN, $1, $3); }
       | expr "^=" expr      { $$ = (AstNode*)createAstBinary(AST_BXOR_ASSIGN, $1, $3); }
       ; 

block   : '{' stmts '}'     { $$ = (AstNode*)createAstBlock(AST_BLOCK, toStaticAstList($2)); }
        ;

stmts   : %empty                { $$ = createDynamicAstList(); }
        | stmts stmt            { $$ = $1; addToDynamicAstList($1, $2); }
        | stmts ';'             { $$ = $1; }
        ;

type    : ID                                 { $$ = (AstNode*)createAstVar($1); }
        | '*' type          %prec UNARY_PRE  { $$ = (AstNode*)createAstUnary(AST_ADDR, $2); }
        | '[' expr ']' type %prec UNARY_PRE  { $$ = (AstNode*)createAstBinary(AST_ARRAY, $2, $4); }
        ;

expr    : ID                            { $$ = (AstNode*)createAstVar($1); }
        | INT                           { $$ = (AstNode*)createAstInt($1); }
        | REAL                          { $$ = (AstNode*)createAstReal($1); }
        | STR                           { $$ = (AstNode*)createAstStr($1); }
        | '(' expr ')'                  { $$ = $2; }
        | '-' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(AST_NEG, $2); }
        | '+' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(AST_POS, $2); }
        | '*' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(AST_ADDR, $2); }
        | '&' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(AST_DEREF, $2); }
        | expr '[' expr ']'             { $$ = (AstNode*)createAstBinary(AST_INDEX, $1, $3); }
        | expr '(' args ')'             { $$ = (AstNode*)createAstCall($1, $3); }
        | expr '+' expr                 { $$ = (AstNode*)createAstBinary(AST_ADD, $1, $3); }
        | expr '-' expr                 { $$ = (AstNode*)createAstBinary(AST_SUB, $1, $3); }
        | expr '*' expr                 { $$ = (AstNode*)createAstBinary(AST_MUL, $1, $3); }
        | expr '/' expr                 { $$ = (AstNode*)createAstBinary(AST_DIV, $1, $3); }
        | expr '%' expr                 { $$ = (AstNode*)createAstBinary(AST_MOD, $1, $3); }
        | expr '&' expr                 { $$ = (AstNode*)createAstBinary(AST_BAND, $1, $3); }
        | expr '|' expr                 { $$ = (AstNode*)createAstBinary(AST_BOR, $1, $3); }
        | expr '^' expr                 { $$ = (AstNode*)createAstBinary(AST_BXOR, $1, $3);}
        | expr "&&" expr                { $$ = (AstNode*)createAstBinary(AST_AND, $1, $3); }
        | expr "||" expr                { $$ = (AstNode*)createAstBinary(AST_OR, $1, $3); }
        | expr ">>" expr                { $$ = (AstNode*)createAstBinary(AST_SHR, $1, $3); }
        | expr "<<" expr                { $$ = (AstNode*)createAstBinary(AST_SHL, $1, $3); }
        | expr "==" expr                { $$ = (AstNode*)createAstBinary(AST_EQ, $1, $3); }
        | expr "!=" expr                { $$ = (AstNode*)createAstBinary(AST_NE, $1, $3); }
        | expr "<=" expr                { $$ = (AstNode*)createAstBinary(AST_LE, $1, $3); }
        | expr ">=" expr                { $$ = (AstNode*)createAstBinary(AST_GE, $1, $3); }
        | expr '>' expr                 { $$ = (AstNode*)createAstBinary(AST_GT, $1, $3); }
        | expr '<' expr                 { $$ = (AstNode*)createAstBinary(AST_LT, $1, $3); }
        ;

args    : %empty    { $$ = createAstList(AST_LIST, 0, NULL); }
        | args_list { $$ = toStaticAstList($1); }
        ;

args_list : expr                { $$ = createDynamicAstList(); addToDynamicAstList($$, $1); }
          | args_list ',' expr  { $$ = $1; addToDynamicAstList($1, $3); }
          ;

%%

