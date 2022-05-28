
%option noinput nounput noyywrap 8bit nodefault
%option reentrant bison-bridge bison-locations

%{
#include "text/string.h"
#include "parser/parser.tab.h"
#include "errors/fatalerror.h"

#define YY_EXTRA_TYPE ParserContext*
#define PARSER_CONTEXT yyget_extra(yyscanner)

#define YY_FATAL_ERROR(MSG) fatalError(str(MSG))

#define YY_USER_ACTION                          \
    yylloc->file = PARSER_CONTEXT->file;        \
    yylloc->offset = getSpanEndOffset(*yylloc); \
    yylloc->length = yyleng;
%}

DIGIT   [0-9]
ID      [_[:alpha:]][_[:alnum:]]*
STR     \"([^"\\]|\\.)*\"
INT     {DIGIT}+
REAL    {DIGIT}+(\.{DIGIT}+)?([eE][-+]?{DIGIT}+)?

%%

[[:space:]]+    { /* Skip all whitespace */ }

"if"            { return IF; }
"else"          { return ELSE; }
"for"           { return FOR; }
"while"         { return WHILE; }
"fn"            { return FN; }
"export"        { return EXPORT; }
"import"        { return IMPORT; }
"type"          { return TYPE; }
"let"           { return LET; }
"return"        { return RETURN; }
"match"         { return MATCH; }
"const"         { return CONST; }

"=="            { return EQ; }
"!="            { return NE; }
"<="            { return LE; }
">="            { return GE; }
">"             { return '>'; }
"<"             { return '<'; }
"||"            { return OR; }
"&&"            { return AND; }

"+="            { return ADD_EQ; }
"-="            { return SUB_EQ; }
"*="            { return MUL_EQ; }
"/="            { return DIV_EQ; }
"%="            { return MOD_EQ; }
">>="           { return SHR_EQ; }
"<<="           { return SHL_EQ; }
"|="            { return BOR_EQ; }
"&="            { return BAND_EQ; }
"^="            { return BXOR_EQ; }
"="             { return '='; }

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"%"             { return '%'; }
"&"             { return '&'; }
"|"             { return '|'; }
"<<"            { return SHL; }
">>"            { return SHR; }

"=>"            { return ARROW; }
":"             { return ':'; }
";"             { return ';'; }
","             { return ','; }
"("             { return '('; }
")"             { return ')'; }
"["             { return '['; }
"]"             { return ']'; }
"{"             { return '{'; }
"}"             { return '}'; }

{INT}           { yylval->lexeme = copyFromCString(yytext); return INT; }

{REAL}          { yylval->lexeme = copyFromCString(yytext); return REAL; }

{STR}           { yylval->lexeme = copyFromCString(yytext); return STR; }

{ID}            { yylval->lexeme = copyFromCString(yytext); return ID; }

.               { return YYUNDEF; }

%%

