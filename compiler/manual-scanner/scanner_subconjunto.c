/*
   1. COMO COMPILAR E EXECUTAR (Linux/WSL)
     gcc scanner.c scanner_subconjunto.c -o scanner_sub
     ./scanner_sub sub_valido.cm
     ./scanner_sub sub_invalido.cm

   2. SAIDA
     A analise e impressa NA TELA e SALVA EM ARQUIVO.
*/

#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>

static const char *nome_tipo[] = {
    "ID", "NUM", "KEYWORD", "SYMBOL", "ERRO", "EOF"
};

int main(int argc, char *argv[])
{
    FILE *arq, *saida;
    char nome_saida[512];
    Token t;
    int linha = 1;
    int n_tokens = 0;
    int n_erros = 0;

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
        if (t.tipo == LEX_EOF) {
            printf("Linha %3d | %-7s | %s\n",
                   t.linha, nome_tipo[LEX_EOF], "fim do arquivo");
            fprintf(saida, "Linha %3d | %-7s | %s\n",
                    t.linha, nome_tipo[LEX_EOF], "fim do arquivo");
            break;
        }

        if (t.tipo == LEX_ERRO) {
            n_erros++;
            printf("Linha %3d | %-7s | caractere invalido: '%s'\n",
                   t.linha, nome_tipo[LEX_ERRO], t.lexema);
            fprintf(saida, "Linha %3d | %-7s | caractere invalido: '%s'\n",
                    t.linha, nome_tipo[LEX_ERRO], t.lexema);
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
