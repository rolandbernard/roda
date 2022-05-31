
%code requires {
#include <stdio.h>

#include "parser/wrapper.h"
#include "ast/astlist.h"
#include "parser/literal.h"

typedef void* yyscan_t;
}

%code provides {
extern int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t yyscanner);
extern void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, ParserContext* context, const char* msg);

#define YYLLOC_DEFAULT(Cur, Rhs, N) {                             \
    if (N == 0) {                                                 \
        (Cur).file = context->file;                               \
        (Cur).begin = YYRHSLOC(Rhs, 0).end;                       \
        (Cur).end = YYRHSLOC(Rhs, 0).end;                         \
    } else {                                                      \
        (Cur) = combineSpans(YYRHSLOC(Rhs, 1), YYRHSLOC(Rhs, N)); \
    }                                                             \
}
}

%define parse.error custom
%define api.pure full
%define api.location.type {Span}
%locations

%lex-param { yyscan_t scanner }
%parse-param { yyscan_t scanner } { ParserContext* context }

%union {
    const char* lexeme;
    AstVar* ident;
    AstNode* ast;
    AstList* list;
    DynamicAstList* dynlist;
}

%destructor { freeAstNode($$); } <ast>
%destructor { freeAstNode((AstNode*)$$); } <ident>
%destructor { freeAstNode((AstNode*)$$); } <list>
%destructor { freeDynamicAstList($$); } <dynlist>

%type <ast> root block stmt block_stmt type expr root_stmt arg_def opt_type assign integer real string
%type <ident> ident
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

program : root  { context->result = $1; }
        ;

root : root_stmts  { $$ = (AstNode*)createAstRoot(@$, toStaticAstList($1)); }
     ;

root_stmts : %empty                     { $$ = createDynamicAstList(); $$->location = @$; }
           | root_stmts root_stmt       { $$ = $1; addToDynamicAstList($1, $2); $$->location = @$; }
           | root_stmts ';'             { $$ = $1; }
           ;

root_stmt : error                                                   { $$ = createAstSimple(@$, AST_ERROR); }
          | "export" "fn" ident '(' args_defs ')' opt_type block    { $$ = (AstNode*)createAstFn(@$, $3, $5, $7, $8, AST_FN_FLAG_EXPORT); }
          | "import" "fn" ident '(' args_defs ')' opt_type ';'      { $$ = (AstNode*)createAstFn(@$, $3, $5, $7, NULL, AST_FN_FLAG_IMPORT); }
          | "fn" ident '(' args_defs ')' opt_type block             { $$ = (AstNode*)createAstFn(@$, $2, $4, $6, $7, AST_FN_FLAG_NONE); }
          | "type" ident '=' type ';'                               { $$ = (AstNode*)createAstTypeDef(@$, $2, $4); }
          ;

opt_type : %empty   { $$ = NULL; }
         | ':' type { $$ = $2; }
         ;

args_defs : %empty              { $$ = createAstList(@$, AST_LIST, 0, NULL); }
          | args_def_list       { $$ = toStaticAstList($1); }
          | args_def_list ','   { $$ = toStaticAstList($1); }
          ;

args_def_list : arg_def                         { $$ = createDynamicAstList(); addToDynamicAstList($$, $1); $$->location = @$; }
              | args_def_list ',' arg_def       { $$ = $1; addToDynamicAstList($1, $3); $$->location = @$; }
              ;

arg_def : error          { $$ = createAstSimple(@$, AST_ERROR); }
        | ident ':' type { $$ = (AstNode*)createAstArgDef(@$, $1, $3); }
        ;

block   : '{' stmts '}'  { $$ = (AstNode*)createAstBlock(@$, toStaticAstList($2)); }
        ;

stmts   : %empty            { $$ = createDynamicAstList(); $$->location = @$; }
        | stmts block_stmt  { $$ = $1; addToDynamicAstList($1, $2); $$->location = @$; }
        | stmts stmt ';'    { $$ = $1; addToDynamicAstList($1, $2); $$->location = @$; }
        | stmts ';'         { $$ = $1; }
        ;

block_stmt  : error                             { $$ = createAstSimple(@$, AST_ERROR); }
            | "if" expr block                   { $$ = (AstNode*)createAstIfElse(@$, $2, $3, NULL); }
            | "if" expr block "else" block      { $$ = (AstNode*)createAstIfElse(@$, $2, $3, $5); }
            | "while" expr block                { $$ = (AstNode*)createAstWhile(@$, $2, $3); }
            | block                             { $$ = $1; }
            ;

stmt    : expr                           { $$ = $1; }
        | assign                         { $$ = $1; }
        | "return"                       { $$ = (AstNode*)createAstUnary(@$, AST_RETURN, NULL); }
        | "return" expr                  { $$ = (AstNode*)createAstUnary(@$, AST_RETURN, $2); }
        | "let" ident opt_type '=' expr  { $$ = (AstNode*)createAstVarDef(@$, $2, $3, $5); }
        | "let" ident opt_type           { $$ = (AstNode*)createAstVarDef(@$, $2, $3, NULL); }
        ;

assign : expr '=' expr      { $$ = (AstNode*)createAstBinary(@$, AST_ASSIGN, $1, $3); }
       | expr "+=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_ADD_ASSIGN, $1, $3); }
       | expr "-=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_SUB_ASSIGN, $1, $3); }
       | expr "*=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_MUL_ASSIGN, $1, $3); }
       | expr "/=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_DIV_ASSIGN, $1, $3); }
       | expr "%=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_MOD_ASSIGN, $1, $3); }
       | expr ">>=" expr    { $$ = (AstNode*)createAstBinary(@$, AST_SHR_ASSIGN, $1, $3); }
       | expr "<<=" expr    { $$ = (AstNode*)createAstBinary(@$, AST_SHL_ASSIGN, $1, $3); }
       | expr "|=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_BOR_ASSIGN, $1, $3); }
       | expr "&=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_BAND_ASSIGN, $1, $3); }
       | expr "^=" expr     { $$ = (AstNode*)createAstBinary(@$, AST_BXOR_ASSIGN, $1, $3); }
       ; 

type    : ident                              { $$ = (AstNode*)$1; }
        | '*' type          %prec UNARY_PRE  { $$ = (AstNode*)createAstUnary(@$, AST_ADDR, $2); }
        | '[' expr ']' type %prec UNARY_PRE  { $$ = (AstNode*)createAstBinary(@$, AST_ARRAY, $2, $4); }
        ;

expr    : ident                         { $$ = (AstNode*)$1; }
        | integer                       { $$ = $1; }
        | real                          { $$ = $1; }
        | string                        { $$ = $1; }
        | '(' expr ')'                  { $$ = $2; }
        | '(' error ')'                 { $$ = createAstSimple(@$, AST_ERROR); }
        | '-' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_NEG, $2); }
        | '+' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_POS, $2); }
        | '*' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_ADDR, $2); }
        | '&' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_DEREF, $2); }
        | expr '[' expr ']'             { $$ = (AstNode*)createAstBinary(@$, AST_INDEX, $1, $3); }
        | expr '(' args ')'             { $$ = (AstNode*)createAstCall(@$, $1, $3); }
        | expr '+' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_ADD, $1, $3); }
        | expr '-' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_SUB, $1, $3); }
        | expr '*' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_MUL, $1, $3); }
        | expr '/' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_DIV, $1, $3); }
        | expr '%' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_MOD, $1, $3); }
        | expr '&' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_BAND, $1, $3); }
        | expr '|' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_BOR, $1, $3); }
        | expr '^' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_BXOR, $1, $3);}
        | expr "&&" expr                { $$ = (AstNode*)createAstBinary(@$, AST_AND, $1, $3); }
        | expr "||" expr                { $$ = (AstNode*)createAstBinary(@$, AST_OR, $1, $3); }
        | expr ">>" expr                { $$ = (AstNode*)createAstBinary(@$, AST_SHR, $1, $3); }
        | expr "<<" expr                { $$ = (AstNode*)createAstBinary(@$, AST_SHL, $1, $3); }
        | expr "==" expr                { $$ = (AstNode*)createAstBinary(@$, AST_EQ, $1, $3); }
        | expr "!=" expr                { $$ = (AstNode*)createAstBinary(@$, AST_NE, $1, $3); }
        | expr "<=" expr                { $$ = (AstNode*)createAstBinary(@$, AST_LE, $1, $3); }
        | expr ">=" expr                { $$ = (AstNode*)createAstBinary(@$, AST_GE, $1, $3); }
        | expr '>' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_GT, $1, $3); }
        | expr '<' expr                 { $$ = (AstNode*)createAstBinary(@$, AST_LT, $1, $3); }
        ;

integer : INT   { $$ = parseIntLiteralIn(context, @1, $1); }
        ;

real    : REAL  { $$ = parseRealLiteralIn(context, @1, $1); }
        ;

string  : STR   { $$ = parseStringLiteralIn(context, @1, $1); }
        ;

ident   : ID    { $$ = createAstVar(@$, getSymbol(&context->context->syms, str($1))); }
        ;

args    : %empty        { $$ = createAstList(@$, AST_LIST, 0, NULL); }
        | args_list     { $$ = toStaticAstList($1); }
        | args_list ',' { $$ = toStaticAstList($1); }
        ;

args_list : error               { $$ = createDynamicAstList(); addToDynamicAstList($$, createAstSimple(@$, AST_ERROR)); $$->location = @$; }
          | expr                { $$ = createDynamicAstList(); addToDynamicAstList($$, $1); $$->location = @$; }
          | args_list ',' expr  { $$ = $1; addToDynamicAstList($1, $3); $$->location = @$; }
          ;

%%

static int yyreport_syntax_error(const yypcontext_t *ctx, yyscan_t scanner, ParserContext* context) {
    yysymbol_kind_t expected[5];
    int count = yypcontext_expected_tokens(ctx, expected, 5);
    const char* exp_names[5];
    for (int i = 0; i < count; i++) {
        exp_names[i] = yysymbol_name(expected[i]);
    }
    reportSyntaxError(context, *yypcontext_location(ctx), yysymbol_name(yypcontext_token(ctx)), count, exp_names);
    return 0;
}

