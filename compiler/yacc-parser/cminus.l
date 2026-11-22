%{
#include "y.tab.h"
#include <string.h>
%}

%option noyywrap
%option yylineno

DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}+
NUM         {DIGIT}+
WHITESPACE  [ \t\r\n]+

%x COMMENT

%%

"else"      { return ELSE; }
"if"        { return IF; }
"int"       { return INT; }
"return"    { return RETURN; }
"void"      { return VOID; }
"while"     { return WHILE; }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return TIMES; }
"/"         { return OVER; }
"<="        { return LE; }
"<"         { return LT; }
">="        { return GE; }
">"         { return GT; }
"=="        { return EQ; }
"!="        { return NE; }
"="         { return ASSIGN; }
";"         { return SEMI; }
","         { return COMMA; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"["         { return LBRACK; }
"]"         { return RBRACK; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }

{ID}        { yylval.str = strdup(yytext); return ID; }
{NUM}       { yylval.str = strdup(yytext); return NUM; }

"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>.|\n       { /* Ignora conteúdo de comentário */ }
<COMMENT><<EOF>>    { fprintf(stderr, "Erro léxico: comentário não fechado.\n"); return 0; }

{WHITESPACE}        { /* Ignora espaços em branco */ }

.           { fprintf(stderr, "Erro léxico na linha %d: caractere inválido '%s'\n", yylineno, yytext); }

%%