/*
   1. COMO COMPILAR E EXECUTAR (Linux/WSL)
     gcc scanner_subconjunto.c -o scanner_sub
     ./scanner_sub sub_valido.cm       (cria saida_sub_valido.cm.txt)
     ./scanner_sub sub_invalido.cm     (cria saida_sub_invalido.cm.txt)

   2. SAIDA
     A analise e impressa NA TELA (printf) e SALVA EM ARQUIVO (fprintf)
     no arquivo "saida_<nome-da-entrada>.txt", com um resumo final:
     total de tokens validos e total de erros lexicos.
     O programa devolve exit code 1 se houve erro lexico.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    S0,   /* estado INICIAL: ainda nao comecou nenhum token            */
    S1,   /* estado de leitura de ID: ja leu 1+ letras (loop letter*)  */
    S2    /* estado de leitura de NUM: ja leu 1+ digitos (loop digit*) */
} Estado;

typedef enum {
    T_ID, T_NUM, T_KEYWORD, T_SYMBOL, T_ERRO, T_EOF
} TipoToken;
const char *nome_tipo[] = { "ID", "NUM", "KEYWORD", "SYMBOL", "ERRO", "EOF" };

typedef struct {
    TipoToken tipo;
    char      lexema[256];
    int       linha;
} Token;

static const char *keywords[] = { "int", "void", NULL };

static int eh_simbolo(int c)
{
    return c == ';' || c == '[' || c == ']';
}

/* Função para scannear tokens individualmente */
Token proximo_token(FILE *f, int *linha)
{
    Token t = { T_EOF, "", *linha };
    Estado e = S0;
    int c;                             /* caractere corrente lido            */
    int i = 0;                         /* indice no buffer do lexema         */
    int k;                             /* indice da tabela de keywords       */

    /* Este loop só termina quando encontrar um caractere 
    que pode iniciar um token, ou o fim do arquivo. */
    for (;;) {
        c = fgetc(f);
        if (c == EOF) return t;
        if (c == '\n') { (*linha)++; continue; }
        if (c == ' ' || c == '\t' || c == '\r') continue;
        break;
    }

    t.linha = *linha;

    if (eh_simbolo(c)) {
        t.tipo      = T_SYMBOL;
        t.lexema[0] = (char)c;
        t.lexema[1] = '\0';
        return t;
    }

    /* S0 --> S1 */
    if (isalpha(c)) {
        e = S1;
        t.lexema[i++] = (char)c;

        while ((c = fgetc(f)) != EOF && isalpha(c)) {
            if (i < 255) t.lexema[i++] = (char)c;
        }
        t.lexema[i] = '\0';

        if (c != EOF) ungetc(c, f);

        t.tipo = T_ID;
        for (k = 0; keywords[k] != NULL; k++) {
            if (strcmp(t.lexema, keywords[k]) == 0) {
                t.tipo = T_KEYWORD;
                break;
            }
        }
        return t;
    }

    /* S0 --> S2 */
    if (isdigit(c)) {
        e = S2;
        t.lexema[i++] = (char)c;

        while ((c = fgetc(f)) != EOF && isdigit(c)) {
            if (i < 255) t.lexema[i++] = (char)c;
        }
        t.lexema[i] = '\0';

        if (c != EOF) ungetc(c, f);

        t.tipo = T_NUM;
        return t;
    }

    /* Erro */
    t.tipo      = T_ERRO;
    t.lexema[0] = (char)c;
    t.lexema[1] = '\0';
    return t;
}

/* Função do fluxo principal */
int main(int argc, char *argv[])
{
    FILE *arq, *saida;
    char  nome_saida[512];
    Token t;
    int   linha = 1;
    int   n_tokens = 0;
    int   n_erros = 0;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte.cm>\n", argv[0]);
        return 1;
    }
    arq = fopen(argv[1], "r");
    if (arq == NULL) {
        perror(argv[1]);
        return 1;
    }
    snprintf(nome_saida, sizeof(nome_saida), "saida_%s.txt", argv[1]);
    saida = fopen(nome_saida, "w");
    if (saida == NULL) {
        perror(nome_saida);
        fclose(arq);
        return 1;
    }

    printf("=== Scanner do subconjunto C- : %s ===\n\n", argv[1]);
    fprintf(saida, "=== Scanner do subconjunto C- : %s ===\n\n", argv[1]);

    for (;;) {
        t = proximo_token(arq, &linha);
        if (t.tipo == T_EOF) {
            printf("Linha %3d | %-7s | %s\n",
                   t.linha, nome_tipo[T_EOF], "fim do arquivo");
            fprintf(saida, "Linha %3d | %-7s | %s\n",
                    t.linha, nome_tipo[T_EOF], "fim do arquivo");
            break;
        }

        if (t.tipo == T_ERRO) {
            n_erros++;
            printf("Linha %3d | %-7s | caractere invalido: '%s'\n",
                   t.linha, nome_tipo[T_ERRO], t.lexema);
            fprintf(saida, "Linha %3d | %-7s | caractere invalido: '%s'\n",
                    t.linha, nome_tipo[T_ERRO], t.lexema);
        } else {
            n_tokens++;
            printf("Linha %3d | %-7s | %s\n",
                   t.linha, nome_tipo[t.tipo], t.lexema);
            fprintf(saida, "Linha %3d | %-7s | %s\n",
                    t.linha, nome_tipo[t.tipo], t.lexema);
        }
    }
    printf("\n=== Resumo ===\n");
    printf("Tokens validos reconhecidos : %d\n", n_tokens);
    printf("Erros lexicos encontrados  : %d\n", n_erros);
    printf("Log salvo em: %s\n", nome_saida);

    fprintf(saida, "\n=== Resumo ===\n");
    fprintf(saida, "Tokens validos reconhecidos : %d\n", n_tokens);
    fprintf(saida, "Erros lexicos encontrados  : %d\n", n_erros);
    fclose(arq);
    fclose(saida);
    return (n_erros > 0) ? 1 : 0;
}
