
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
    char* lexeme;
    AstVar* var;
    AstNode* ast;
    struct {
        AstList* list;
        AstFnFlags flags;
    } arg_defs;
    AstList* list;
}

%destructor { freeTemporaryString(context, $$); } <lexeme>
%destructor { freeAstNode((AstNode*)$$); } <var>
%destructor { freeAstNode($$); } <ast>
%destructor { freeAstNode((AstNode*)$$.list); } <arg_defs>
%destructor { freeAstNode((AstNode*)$$); } <list>

%type <ast> root block stmt block_stmt type expr root_stmt if
%type <ast> arg_def opt_type assign integer real string bool field_val
%type <var> var
%type <arg_defs> arg_defs arg_types
%type <list> arg_def_list stmts root_stmts list
%type <list> type_list_nonempty list_nonempty field_vals arg_type_list

%token <lexeme> ID      "identifier"
%token <lexeme> STR     "string"
%token <lexeme> INT     "integer"
%token <lexeme> REAL    "real"
%token <lexeme> CHAR    "character"
%token TRUE             "true"
%token FALSE            "false"

%token IF               "if"
%token ELSE             "else"
%token WHILE            "while"
%token TYPE             "type"
%token LET              "let"
%token FN               "fn"
%token RETURN           "return"
%token EXTERN           "extern"
%token PUB              "pub"
%token SIZEOF           "sizeof"
%token AS               "as"
%token EQ               "=="
%token NE               "!="
%token LE               "<="
%token GE               ">="
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
%token DOTS             ".."

%left "||"
%left "&&"
%left '<' '>' "==" "!=" "<=" ">="
%left '|'
%left '^'
%left '&'
%left "<<" ">>"
%left '-' '+'
%left '*' '/' '%'
%precedence "as"
%precedence UNARY_PRE
%precedence '(' '['
%precedence '.'

%start program

%%

program : root  { context->result = $1; }
        ;

root : root_stmts  { fitAstList($1); $$ = (AstNode*)createAstRoot(@$, $1); }
     ;

root_stmts : %empty                     { $$ = createEmptyAstList(); $$->location = @$; }
           | root_stmts root_stmt       { $$ = $1; addToAstList($1, $2); $$->location = @$; }
           | root_stmts ';'             { $$ = $1; }
           ;

root_stmt : error                                                   { $$ = createAstSimple(@$, AST_ERROR); }
          | "pub" "fn" var '(' arg_defs ')' opt_type block          { $$ = (AstNode*)createAstFn(@$, $3, $5.list, $7, $8, AST_FN_FLAG_EXPORT | $5.flags); }
          | "extern" "fn" var '(' arg_defs ')' opt_type ';'         { $$ = (AstNode*)createAstFn(@$, $3, $5.list, $7, NULL, AST_FN_FLAG_IMPORT | $5.flags); }
          | "fn" var '(' arg_defs ')' opt_type block                { $$ = (AstNode*)createAstFn(@$, $2, $4.list, $6, $7, $4.flags); }
          | "type" var '=' type ';'                                 { $$ = (AstNode*)createAstTypeDef(@$, $2, $4); }
          ;

opt_type : %empty   { $$ = NULL; }
         | ':' type { $$ = $2; }
         ;

arg_defs : %empty                 { $$.list = createAstList(@$, AST_LIST, 0, NULL); $$.flags = AST_FN_FLAG_NONE; }
         | arg_def_list           { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
         | arg_def_list ','       { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
         | arg_def_list ',' ".."  { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_VARARG; }
         ;

arg_def_list : arg_def                      { $$ = createEmptyAstList(); addToAstList($$, $1); $$->location = @$; }
             | arg_def_list ',' arg_def     { $$ = $1; addToAstList($1, $3); $$->location = @$; }
             ;

arg_def : error             { $$ = createAstSimple(@$, AST_ERROR); }
        | var ':' type      { $$ = (AstNode*)createAstArgDef(@$, $1, $3); }
        ;

block   : '{' stmts '}'  { fitAstList($2); $$ = (AstNode*)createAstBlock(@$, $2); }
        ;

stmts   : %empty            { $$ = createEmptyAstList(); $$->location = @$; }
        | stmts block_stmt  { $$ = $1; addToAstList($1, $2); $$->location = @$; }
        | stmts stmt ';'    { $$ = $1; addToAstList($1, $2); $$->location = @$; }
        | stmts ';'         { $$ = $1; }
        ;

block_stmt  : error                             { $$ = createAstSimple(@$, AST_ERROR); }
            | "while" expr block                { $$ = (AstNode*)createAstWhile(@$, $2, $3); }
            | block                             { $$ = $1; }
            | if                                { $$ = $1; }
            ;

if  : "if" expr block                  { $$ = (AstNode*)createAstIfElse(@$, $2, $3, NULL); }
    | "if" expr block "else" block     { $$ = (AstNode*)createAstIfElse(@$, $2, $3, $5); }
    | "if" expr block "else" if        { $$ = (AstNode*)createAstIfElse(@$, $2, $3, $5); }
    ;

stmt    : expr                              { $$ = $1; }
        | assign                            { $$ = $1; }
        | "return"                          { $$ = (AstNode*)createAstReturn(@$, NULL); }
        | "return" expr                     { $$ = (AstNode*)createAstReturn(@$, $2); }
        | "let" var opt_type '=' expr       { $$ = (AstNode*)createAstVarDef(@$, $2, $3, $5); }
        | "let" var opt_type                { $$ = (AstNode*)createAstVarDef(@$, $2, $3, NULL); }
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

type    : var                                   { $$ = (AstNode*)$1; }
        | '(' ')'                               { $$ = createAstSimple(@$, AST_VOID); }
        | '(' arg_def_list ')'                  { fitAstList($2); $$ = (AstNode*)$2; $$->kind = AST_STRUCT_TYPE; $$->location = @$; }
        | '(' arg_def_list ',' ')'              { fitAstList($2); $$ = (AstNode*)$2; $$->kind = AST_STRUCT_TYPE; $$->location = @$; }
        | '*' type          %prec UNARY_PRE     { $$ = (AstNode*)createAstUnary(@$, AST_ADDR, $2); }
        | '[' expr ']' type %prec UNARY_PRE     { $$ = (AstNode*)createAstBinary(@$, AST_ARRAY, $2, $4); }
        | "fn" '(' arg_types ')'                { $$ = (AstNode*)createAstFnType(@$, $3.list, NULL, $3.flags != AST_FN_FLAG_NONE); }
        | "fn" '(' arg_types ')' ':' type       { $$ = (AstNode*)createAstFnType(@$, $3.list, $6, $3.flags != AST_FN_FLAG_NONE); }
        ;

arg_types : %empty                          { $$.list = createAstList(@$, AST_LIST, 0, NULL); $$.flags = AST_FN_FLAG_NONE; }
          | type_list_nonempty              { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
          | type_list_nonempty ','          { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
          | type_list_nonempty ',' ".."     { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_VARARG; }
          | arg_type_list                   { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
          | arg_type_list ','               { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_NONE; }
          | arg_type_list ',' ".."          { fitAstList($1); $$.list = $1; $$.flags = AST_FN_FLAG_VARARG; }
          ;

type_list_nonempty  : type                          { $$ = createEmptyAstList(); addToAstList($$, $1); $$->location = @$; }
                    | type_list_nonempty ',' type   { $$ = $1; addToAstList($1, $3); $$->location = @$; }
                    ;

arg_type_list   : ident ':' type                       { $$ = createEmptyAstList(); addToAstList($$, $3); $$->location = @$; }
                | arg_type_list ',' ident ':' type     { $$ = $1; addToAstList($1, $5); $$->location = @$; }
                ;

expr    : var                           { $$ = (AstNode*)$1; }
        | integer                       { $$ = $1; }
        | real                          { $$ = $1; }
        | string                        { $$ = $1; }
        | bool                          { $$ = $1; }
        | expr '.' var                  { $$ = (AstNode*)createAstStructIndex(@$, $1, $3); }
        | "sizeof" type                 { $$ = (AstNode*)createAstUnary(@$, AST_SIZEOF, $2); }
        | '(' ')'                       { $$ = createAstSimple(@$, AST_VOID); }
        | '[' list ']'                  { fitAstList($2); $$ = (AstNode*)$2; $$->kind = AST_ARRAY_LIT; $$->location = @$; }
        | '(' expr ')'                  { $$ = $2; }
        | '(' field_vals ')'            { fitAstList($2); $$ = (AstNode*)$2; $$->kind = AST_STRUCT_LIT; $$->location = @$; }
        | '(' field_vals ',' ')'        { fitAstList($2); $$ = (AstNode*)$2; $$->kind = AST_STRUCT_LIT; $$->location = @$; }
        | '-' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_NEG, $2); }
        | '+' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_POS, $2); }
        | '*' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_ADDR, $2); }
        | '&' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_DEREF, $2); }
        | '!' expr %prec UNARY_PRE      { $$ = (AstNode*)createAstUnary(@$, AST_NOT, $2); }
        | expr '[' expr ']'             { $$ = (AstNode*)createAstBinary(@$, AST_INDEX, $1, $3); }
        | expr '(' list ')'             { fitAstList($3); $$ = (AstNode*)createAstCall(@$, $1, $3); }
        | expr "as" type                { $$ = (AstNode*)createAstBinary(@$, AST_AS, $1, $3); }
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
        | '[' error ']'                 { $$ = createAstSimple(@$, AST_ERROR); }
        | '(' error ')'                 { $$ = createAstSimple(@$, AST_ERROR); }
        | expr '[' error ']'            { $$ = createAstSimple(@$, AST_ERROR); freeAstNode($1); }
        | expr '(' error ')'            { $$ = createAstSimple(@$, AST_ERROR); freeAstNode($1); }
        ;

field_vals  : field_val                     { $$ = createEmptyAstList(); addToAstList($$, $1); $$->location = @$; }
            | field_vals ',' field_val      { $$ = $1; addToAstList($1, $3); $$->location = @$; }
            ;

field_val : var '=' expr    { $$ = (AstNode*)createAstArgDef(@$, $1, $3); }
          ;

list    : %empty                { $$ = createEmptyAstList(); $$->location = @$; }
        | list_nonempty         { $$ = $1; }
        | list_nonempty ','     { $$ = $1; }
        ;

list_nonempty : expr                    { $$ = createEmptyAstList(); addToAstList($$, $1);; $$->location = @$; }
              | list_nonempty ',' expr  { $$ = $1; addToAstList($$, $3); $$->location = @$; }
              ;

integer : INT   { $$ = parseIntLiteralIn(context, @1, $1); freeTemporaryString(context, $1); }
        | CHAR  { $$ = parseCharLiteralIn(context, @1, $1); freeTemporaryString(context, $1); }
        ;

real    : REAL  { $$ = parseRealLiteralIn(context, @1, $1); freeTemporaryString(context, $1); }
        ;

string  : STR   { $$ = parseStringLiteralIn(context, @1, $1); freeTemporaryString(context, $1); }
        ;

var     : ID    { $$ = createAstVar(@$, getSymbol(&context->context->syms, str($1))); freeTemporaryString(context, $1); }
        ;

ident   : ID    { freeTemporaryString(context, $1); }
        ;

bool    : "true"    { $$ = (AstNode*)createAstBool(@$, true); }
        | "false"   { $$ = (AstNode*)createAstBool(@$, false); }
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

