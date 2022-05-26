
%define parse.error verbose

%union {
    const char* lexeme;
}

%token <lexeme> ID
%token <lexeme> STR
%token <lexeme> INT
%token <lexeme> REAL
%token IF
%token ELSE
%token FOR
%token WHILE
%token FN
%token EXPORT
%token IMPORT
%token TYPE
%token LET
%token RETURN
%token EQ
%token NE
%token LE
%token GE
%token CONST
%token ARROW
%token MATCH
%token ADD_EQ
%token SUB_EQ
%token MUL_EQ
%token DIV_EQ
%token MOD_EQ
%token SHR_EQ
%token SHL_EQ
%token BOR_EQ
%token BAND_EQ
%token BXOR_EQ
%token OR
%token AND
%token SHR
%token SHL

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

%start root

%%

root    : expr      {}
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

expr    : ID                                    {}
        | INT                                   {}
        | REAL                                  {}
        | STR                                   {}
        | '(' expr ')'                          {}
        | '-' expr %prec UNARY_PRE              {}
        | '+' expr %prec UNARY_PRE              {}
        | '*' expr %prec UNARY_PRE              {}
        | '&' expr %prec UNARY_PRE              {}
        | expr '[' expr ']' %prec UNARY_POST    {}
        | expr '(' args ')' %prec UNARY_POST    {}
        | expr '+' expr                         {}
        | expr '-' expr                         {}
        | expr '*' expr                         {}
        | expr '/' expr                         {}
        | expr '%' expr                         {}
        | expr '&' expr                         {}
        | expr '|' expr                         {}
        | expr '^' expr                         {}
        | expr AND expr                         {}
        | expr OR expr                          {}
        | expr SHR expr                         {}
        | expr SHL expr                         {}
        | expr EQ expr                          {}
        | expr NE expr                          {}
        | expr LE expr                          {}
        | expr GE expr                          {}
        | expr '>' expr                         {}
        | expr '<' expr                         {}
        | IF expr block                         {}
        | IF expr block ELSE block              {}
        ;

args    : %empty
        ;
%%

void yyerror(const char* s) {
    // TODO
    printf("ERROR %s\n", s);
}

