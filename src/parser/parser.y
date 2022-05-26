
%{
#define YYERROR_VERBOSE 1
%}

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

%start module

%%

module  : ID { printf("ID %s\n", $1); }
        | STR { printf("STR %s\n", $1); }
        ;

%%

void yyerror(const char* s) {
    // TODO
    printf("ERROR %s\n", s);
}

