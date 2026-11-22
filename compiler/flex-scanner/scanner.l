%{
#ifdef YACC_PARSER
#include "y.tab.h"
#include <stdio.h>
#include <string.h>
int lexical_error = 0;
#else
#include <stdio.h>
#include <string.h>
int linha_standalone = 1;
int n_tokens_standalone = 0;
int n_erros_standalone = 0;
FILE *saida_standalone;
#endif
%}

%option noyywrap
%option yylineno

LETTER      [A-Za-z]
DIGIT       [0-9]
ID          {LETTER}({LETTER}|{DIGIT})*
NUM         {DIGIT}+
WHITESPACE  [ \t\r\n]+

%x COMMENT

%%

"else"      {
#ifdef YACC_PARSER
    return ELSE;
#else
    printf("Linha %3d | KEYWORD | else\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | else\n", linha_standalone);
    n_tokens_standalone++;
#endif
}
"if"        {
#ifdef YACC_PARSER
    return IF;
#else
    printf("Linha %3d | KEYWORD | if\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | if\n", linha_standalone);
    n_tokens_standalone++;
#endif
}
"int"       {
#ifdef YACC_PARSER
    return INT;
#else
    printf("Linha %3d | KEYWORD | int\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | int\n", linha_standalone);
    n_tokens_standalone++;
#endif
}
"return"    {
#ifdef YACC_PARSER
    return RETURN;
#else
    printf("Linha %3d | KEYWORD | return\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | return\n", linha_standalone);
    n_tokens_standalone++;
#endif
}
"void"      {
#ifdef YACC_PARSER
    return VOID;
#else
    printf("Linha %3d | KEYWORD | void\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | void\n", linha_standalone);
    n_tokens_standalone++;
#endif
}
"while"     {
#ifdef YACC_PARSER
    return WHILE;
#else
    printf("Linha %3d | KEYWORD | while\n", linha_standalone);
    fprintf(saida_standalone, "Linha %3d | KEYWORD | while\n", linha_standalone);
    n_tokens_standalone++;
#endif
}

"+"|"-"|"*"|"/"|"<="|"<"|">="|">"|"=="|"!="|"="|";"|","|"("|")"|"["|"]"|"{"|"}" {
#ifdef YACC_PARSER
    if (strcmp(yytext, "+") == 0) return PLUS;
    if (strcmp(yytext, "-") == 0) return MINUS;
    if (strcmp(yytext, "*") == 0) return TIMES;
    if (strcmp(yytext, "/") == 0) return OVER;
    if (strcmp(yytext, "<=") == 0) return LE;
    if (strcmp(yytext, "<") == 0) return LT;
    if (strcmp(yytext, ">=") == 0) return GE;
    if (strcmp(yytext, ">") == 0) return GT;
    if (strcmp(yytext, "==") == 0) return EQ;
    if (strcmp(yytext, "!=") == 0) return NE;
    if (strcmp(yytext, "=") == 0) return ASSIGN;
    if (strcmp(yytext, ";") == 0) return SEMI;
    if (strcmp(yytext, ",") == 0) return COMMA;
    if (strcmp(yytext, "(") == 0) return LPAREN;
    if (strcmp(yytext, ")") == 0) return RPAREN;
    if (strcmp(yytext, "[") == 0) return LBRACK;
    if (strcmp(yytext, "]") == 0) return RBRACK;
    if (strcmp(yytext, "{") == 0) return LBRACE;
    if (strcmp(yytext, "}") == 0) return RBRACE;
#else
    printf("Linha %3d | SYMBOL  | %s\n", linha_standalone, yytext);
    fprintf(saida_standalone, "Linha %3d | SYMBOL  | %s\n",
            linha_standalone, yytext);
    n_tokens_standalone++;
#endif
}

{ID}        {
#ifdef YACC_PARSER
    yylval.str = strdup(yytext);
    return ID;
#else
    printf("Linha %3d | ID      | %s\n", linha_standalone, yytext);
    fprintf(saida_standalone, "Linha %3d | ID      | %s\n", linha_standalone, yytext);
    n_tokens_standalone++;
#endif
}
{NUM}       {
#ifdef YACC_PARSER
    yylval.str = strdup(yytext);
    return NUM;
#else
    printf("Linha %3d | NUM     | %s\n", linha_standalone, yytext);
    fprintf(saida_standalone, "Linha %3d | NUM     | %s\n", linha_standalone, yytext);
    n_tokens_standalone++;
#endif
}

"/*"                    { BEGIN(COMMENT); }
<COMMENT>"*/"           { BEGIN(INITIAL); }
<COMMENT>.|\n           { /* Ignora o conteudo do comentario. */ }
<COMMENT><<EOF>>        {
#ifdef YACC_PARSER
                            fprintf(stderr,
                                    "Erro lexico: comentario nao fechado.\n");
                            lexical_error = 1;
#else
                            fprintf(stdout,
                                    "Erro lexico: comentario nao fechado.\n");
                            n_erros_standalone++;
#endif
                            return 0;
                        }

{WHITESPACE}            {
#ifndef YACC_PARSER
    for (char *p = yytext; *p; p++) {
        if (*p == '\n') linha_standalone++;
    }
#endif
}

.                       {
#ifdef YACC_PARSER
                            fprintf(stderr,
                                    "Erro lexico na linha %d: caractere invalido '%s'\n",
                                    yylineno, yytext);
                            lexical_error = 1;
#else
                            fprintf(stdout,
                                    "Linha %3d | ERRO    | caractere invalido: '%s'\n",
                                    linha_standalone, yytext);
                            fprintf(saida_standalone,
                                    "Linha %3d | ERRO    | caractere invalido: '%s'\n",
                                    linha_standalone, yytext);
                            n_erros_standalone++;
#endif
                        }

%%

#ifndef YACC_PARSER
int main(int argc, char **argv)
{
    char nome_saida[512];
    FILE *entrada;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte.cm>\n", argv[0]);
        return 1;
    }

    entrada = fopen(argv[1], "r");
    if (!entrada) {
        perror(argv[1]);
        return 1;
    }

    snprintf(nome_saida, sizeof(nome_saida), "saida_%s.txt", argv[1]);
    saida_standalone = fopen(nome_saida, "w");
    if (!saida_standalone) {
        perror(nome_saida);
        fclose(entrada);
        return 1;
    }

    yyin = entrada;
    yylex();

    printf("Tokens validos reconhecidos : %d\n", n_tokens_standalone);
    printf("Erros lexicos encontrados  : %d\n", n_erros_standalone);
    fprintf(saida_standalone,
            "Tokens validos reconhecidos : %d\n", n_tokens_standalone);
    fprintf(saida_standalone,
            "Erros lexicos encontrados  : %d\n", n_erros_standalone);

    fclose(entrada);
    fclose(saida_standalone);
    return n_erros_standalone > 0 ? 1 : 0;
}
#endif
