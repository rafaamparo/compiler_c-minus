%{
/* ============================================================
   scanner.l - Analisador lexico para a linguagem C-.
   Baseado em: Louden, "Compiler Construction: Principles and
   Practice" - Linguagem C- (C-minus)

   A versao salva o log completo de cada analise em um arquivo
   chamado saida_<arquivo>.txt, alem de imprimir no terminal.

   Como compilar e executar (Linux/WSL):
     flex scanner.l
     gcc lex.yy.c -o scanner
     ./scanner teste_valido.cm
     ./scanner teste_invalido.cm
   ============================================================ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int linha = 1;
int n_tokens = 0;
int n_erros = 0;
FILE *saida;

void registrar(const char *classe);
void registrar_erro(const char *msg, const char *detalhe);
%}

%option noyywrap

/* ---- Definicoes regulares ---- */
LETTER [A-Za-z]
DIGIT  [0-9]
ID     {LETTER}({LETTER}|{DIGIT})*
NUM    {DIGIT}+

%x COMENTARIO

%%

/* ================== ESTADO INICIAL ================== */
<INITIAL>{
    "/*"  { BEGIN(COMENTARIO); }

    /* Palavras-chave do C- */
    "int"|"void"|"if"|"else"|"while"|"return" {
        registrar("KEYWORD");
    }

    {ID}  { registrar("ID"); }
    {NUM} { registrar("NUM"); }

    /* Simbolos do C-: operadores e delimitadores */
    "<="|">="|"=="|"!="|
    "+"|"-"|"*"|"/"|
    "<"|">"|"="|";"|","|
    "("|")"|"["|"]"|"{"|"}" {
        registrar("SYMBOL");
    }

    \n       { linha++; }
    [ \t\r]+ { /* ignora espacos em branco */ }

    . { registrar_erro("caractere invalido:", yytext); }
}

/* ================== COMENTARIO ================== */
<COMENTARIO>{
    <<EOF>> {
        registrar_erro("comentario '/*' nao foi fechado", "");
        yyterminate();
    }

    "*/" { BEGIN(INITIAL); }
    \n    { linha++; }
    . { /* ignora o conteudo do comentario */ }
}

%%

/* ---------- helpers ---------- */
void registrar(const char *classe)
{
    printf("Linha %3d | %-7s | %s\n", linha, classe, yytext);
    fprintf(saida, "Linha %3d | %-7s | %s\n", linha, classe, yytext);
    n_tokens++;
}

void registrar_erro(const char *msg, const char *detalhe)
{
    n_erros++;
    printf("Linha %3d | %-7s | %s %s\n", linha, "ERRO", msg, detalhe);
    fprintf(saida, "Linha %3d | %-7s | %s %s\n", linha, "ERRO", msg, detalhe);
}

static FILE *abrir_arquivo_entrada(const char *arquivo)
{
    FILE *arq = fopen(arquivo, "r");
    if (arq == NULL) {
        perror(arquivo);
    }
    return arq;
}

static FILE *abrir_arquivo_saida(const char *arquivo_entrada)
{
    char nome_saida[512];
    snprintf(nome_saida, sizeof(nome_saida), "saida_%s.txt", arquivo_entrada);

    FILE *arquivo = fopen(nome_saida, "w");
    if (arquivo == NULL) {
        perror(nome_saida);
    }

    return arquivo;
}

int main(int argc, char *argv[])
{
    FILE *arq;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte.cm>\n", argv[0]);
        return 1;
    }

    arq = abrir_arquivo_entrada(argv[1]);
    if (arq == NULL) {
        return 1;
    }
    yyin = arq;

    saida = abrir_arquivo_saida(argv[1]);
    if (saida == NULL) {
        fclose(arq);
        return 1;
    }

    printf("=== Analise lexica de: %s ===\n", argv[1]);
    fprintf(saida, "=== Analise lexica de: %s ===\n\n", argv[1]);

    yylex();

    printf("\n=== Resumo ===\n");
    printf("Tokens validos reconhecidos : %d\n", n_tokens);
    printf("Erros lexicos encontrados  : %d\n", n_erros);
    printf("Log salvo em: saida_%s.txt\n", argv[1]);

    fprintf(saida, "\n=== Resumo ===\n");
    fprintf(saida, "Tokens validos reconhecidos : %d\n", n_tokens);
    fprintf(saida, "Erros lexicos encontrados  : %d\n", n_erros);

    fclose(arq);
    fclose(saida);

    return (n_erros > 0) ? 1 : 0;
}
