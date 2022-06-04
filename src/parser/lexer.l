
%option noinput nounput noyywrap 8bit nodefault
%option reentrant bison-bridge bison-locations

%{
#include "text/string.h"
#include "parser/parser.tab.h"
#include "errors/fatalerror.h"

#define YY_EXTRA_TYPE ParserContext*
#define PARSER_CONTEXT yyget_extra(yyscanner)

#define YY_FATAL_ERROR(MSG) fatalError(str(MSG))

#define YY_USER_ACTION                                              \
    yylloc->file = PARSER_CONTEXT->file;                            \
    yylloc->begin = yylloc->end;                                    \
    yylloc->end = advanceLocationWith(yylloc->end, yytext, yyleng);
%}

ID      [_[:alpha:]][_[:alnum:]]*
STR     \"([^"\\]|\\.)*\"
INT     0[hx][_0-9a-fA-F]+|0o[_0-7]+|0d[_0-9]+|0b[_01]+|[0-9][0-9_]*
REAL    [0-9][_0-9]*(\.[0-9][_0-9]*)?([eE][-+]?[0-9][_0-9]*)?

UB [\200-\277]
POINT .|[\300-\337]{UB}|[\340-\357]{UB}{2}|[\360-\367]{UB}{3}|[\370-\373]{UB}{4}|[\374-\375]{UB}{5}

%x BLOCK_COMMENT

%%

"/*"            { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>{
  "/*"          { PARSER_CONTEXT->comment_nesting++; }
  "*/"          { if (PARSER_CONTEXT->comment_nesting != 0) { PARSER_CONTEXT->comment_nesting--; } else { BEGIN(INITIAL); } }
  [/*]          { /* Skip + and / characters */ }
  [^/*]+        { /* Skip all other characters block comments */ }
}

"//"[^\n]*      { /* Skip line comments */ }

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
"!"             { return '!'; }
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

{INT}           { yylval->lexeme = yytext; return INT; }

{REAL}          { yylval->lexeme = yytext; return REAL; }

{STR}           { yylval->lexeme = yytext; return STR; }

{ID}            { yylval->lexeme = yytext; return ID; }

{POINT}         { return YYUNDEF; }

%%

