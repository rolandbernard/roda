
%code requires {
#include "ast/ast.h"
#include "ast/astlist.h"
#include <stdio.h>

extern void yyerror(AstNode**, const char*);
extern int yylex();
}

%define parse.error verbose

%parse-param {AstNode** ast_result}

%union {
    const char* lexeme;
    AstNode* ast;
    AstList* list;
    DynamicAstList* dynlist;
}

%destructor { freeAstNode($$); } <ast>
%destructor { freeAstNode((AstNode*)$$); } <list>
%destructor { freeDynamicAstList($$); } <dynlist>

%type <ast> root block stmt stmts type expr
%type <list> args
%type <dynlist> args_list 

%token <lexeme> ID STR INT REAL
%token IF ELSE FOR WHILE MATCH
%token EXPORT IMPORT
%token CONST
%token TYPE LET FN
%token RETURN
%token EQ NE LE GE
%token ARROW
%token ADD_EQ SUB_EQ MUL_EQ DIV_EQ MOD_EQ SHR_EQ SHL_EQ BOR_EQ BAND_EQ BXOR_EQ
%token OR AND SHR SHL

%left OR
%left AND
%left '<' '>' EQ NE LE GE
%left '|'
%left '^'
%left '&'
%left SHL SHR
%left '-' '+'
%left '*' '/' '%'
%precedence UNARY_PRE
%precedence UNARY_POST

%start program

%%


//todo:
// chars
// function definitions
// loops
// match
// top level definitions
// 

program : root  { *ast_result = $1; }
        ;

root    : expr  { $$ = $1; }
        ;

stmt    : expr                          {}
        | RETURN                        {}
        | expr '=' expr                 {}
        | RETURN expr                   {}
        | LET expr '=' expr             {}
        | LET expr ':' type '=' expr    {}
        | LET expr ':' type             {}
        | TYPE ID '=' type              {}
        ;

block   : '{' stmts '}'     {}
        ;

stmts   : %empty
        | stmts stmt ';'
        ;

type    : ID                {}
        | '*' ID            {}
        | '[' expr ']' ID   {}
        ;

expr    : ID                                    { $$ = (AstNode*)createAstVar($1); }
        | INT                                   { $$ = (AstNode*)createAstInt($1); }
        | REAL                                  { $$ = (AstNode*)createAstReal($1); }
        | STR                                   { $$ = (AstNode*)createAstStr($1); }
        | '(' expr ')'                          { $$ = $2; }
        | '-' expr %prec UNARY_PRE              { $$ = (AstNode*)createAstUnary(AST_NEG, $2); }
        | '+' expr %prec UNARY_PRE              { $$ = (AstNode*)createAstUnary(AST_POS, $2); }
        | '*' expr %prec UNARY_PRE              { $$ = (AstNode*)createAstUnary(AST_ADDR, $2); }
        | '&' expr %prec UNARY_PRE              { $$ = (AstNode*)createAstUnary(AST_DEREF, $2); }
        | expr '[' expr ']' %prec UNARY_POST    { $$ = (AstNode*)createAstBinary(AST_INDEX, $1, $3); }
        | expr '(' args ')' %prec UNARY_POST    { $$ = (AstNode*)createAstCall($1, $3); }
        | expr '+' expr                         { $$ = (AstNode*)createAstBinary(AST_ADD, $1, $3); }
        | expr '-' expr                         { $$ = (AstNode*)createAstBinary(AST_SUB, $1, $3); }
        | expr '*' expr                         { $$ = (AstNode*)createAstBinary(AST_MUL, $1, $3); }
        | expr '/' expr                         { $$ = (AstNode*)createAstBinary(AST_DIV, $1, $3); }
        | expr '%' expr                         { $$ = (AstNode*)createAstBinary(AST_MOD, $1, $3); }
        | expr '&' expr                         { $$ = (AstNode*)createAstBinary(AST_BAND, $1, $3); }
        | expr '|' expr                         { $$ = (AstNode*)createAstBinary(AST_BOR, $1, $3); }
        | expr '^' expr                         { $$ = (AstNode*)createAstBinary(AST_BXOR, $1, $3);}
        | expr AND expr                         { $$ = (AstNode*)createAstBinary(AST_AND, $1, $3); }
        | expr OR expr                          { $$ = (AstNode*)createAstBinary(AST_OR, $1, $3); }
        | expr SHR expr                         { $$ = (AstNode*)createAstBinary(AST_SHR, $1, $3); }
        | expr SHL expr                         { $$ = (AstNode*)createAstBinary(AST_SHL, $1, $3); }
        | expr EQ expr                          { $$ = (AstNode*)createAstBinary(AST_EQ, $1, $3); }
        | expr NE expr                          { $$ = (AstNode*)createAstBinary(AST_NE, $1, $3); }
        | expr LE expr                          { $$ = (AstNode*)createAstBinary(AST_LE, $1, $3); }
        | expr GE expr                          { $$ = (AstNode*)createAstBinary(AST_GE, $1, $3); }
        | expr '>' expr                         { $$ = (AstNode*)createAstBinary(AST_GT, $1, $3); }
        | expr '<' expr                         { $$ = (AstNode*)createAstBinary(AST_LT, $1, $3); }
        | IF expr block                         { $$ = (AstNode*)createAstIfElse($2, $3, NULL); }
        | IF expr block ELSE block              { $$ = (AstNode*)createAstIfElse($2, $3, $5); }
        ;

args    : %empty    { $$ = createAstList(AST_LIST, 0, NULL); }
        | args_list { $$ = toStaticAstList($1); }
        ;

args_list : expr                { $$ = createDynamicAstList(); addToDynamicAstList($$, $1); }
          | args_list ',' expr  { $$ = $1; addToDynamicAstList($1, $3); }
          ;

%%

void yyerror(AstNode** ast_result, const char* s) {
    // TODO
    printf("ERROR %s\n", s);
}

