/* ============================================================================
   TRABALHO - PARTE 1: SCANNER (ANALISADOR LEXICO) DO SUBCONJUNTO DO C-
   ============================================================================
   ---------------------------------------------------------------------------
   1. REGRAS DO SUBCONJUNTO ESCOLHIDO (BNF)
   ---------------------------------------------------------------------------
     1) program           := declaration-list
     2) declaration-list   := declaration-list declaration | declaration
     3) declaration        := var-declaration
     4) var-declaration   := type-specifier ID ;
                         |  type-specifier ID [ NUM ] ;
     5) type-specifier    := int | void

   ---------------------------------------------------------------------------
   2. EXPRESSOES REGULARES DOS TOKENS
   ---------------------------------------------------------------------------
     letter   = [a-zA-Z]
     digit    = [0-9]
     keywords = int | void
     ID       = letter letter*        (uma letra, depois zero ou mais letras)
     NUM      = digit digit*          (um digito, depois zero ou mais digitos)
     symbols  = ; | [ | ]

   ---------------------------------------------------------------------------
   3. DFA MINIMO IMPLEMENTADO ABAIXO
      (resultado do processo manual NFA -> DFA -> DFA minimo)
   ---------------------------------------------------------------------------
     Estados:
       S0 : estado inicial (esperando o comeco de um token)
       S1 : leu 1 ou mais letras  -> ao terminar, ACEITA ID ou KEYWORD
       S2 : leu 1 ou mais digitos -> ao terminar, ACEITA NUM

     Tabela de transicoes (resumo):
       +--------+--------+-------+------------+------------------+
       | Estado | letter | digit | ;  [  ]    | outro            |
       +--------+--------+-------+------------+------------------+
       |  S0    |  -> S1 | -> S2 | ACEITA     | ERRO LEXICO      |
       |  S1    |  -> S1 | fim*  | fim*       | fim*             |
       |  S2    |  fim** | -> S2 | fim**      | fim**            |
       +--------+--------+-------+------------+------------------+
       *  fim = aceita o ID em construcao e REPROCESSA o caractere atual
           (volta a S0 devolvendo o caractere lido com ungetc)
       ** fim = aceita o NUM em construcao e reprocessa o caractere

     Observacao sobre keywords: "int" e "void" casam com a ER de ID
     (sao apenas sequencias de letras). Por isso, no DFA minimo os
     estados de aceitacao de ID sao os mesmos, e a distincao final
     entre KEYWORD e ID e feita por uma TABELA DE PALAVRAS RESERVADAS,
     logo apos o token ser aceito (tecnica classica de scanners).

   ---------------------------------------------------------------------------
   4. COMO COMPILAR E EXECUTAR (Linux/WSL)
   ---------------------------------------------------------------------------
     gcc scanner_subconjunto.c -o scanner_sub
     ./scanner_sub sub_valido.cm       (cria saida_sub_valido.cm.txt)
     ./scanner_sub sub_invalido.cm     (cria saida_sub_invalido.cm.txt)

   ---------------------------------------------------------------------------
   5. SAIDA
   ---------------------------------------------------------------------------
     A analise e impressa NA TELA (printf) e SALVA EM ARQUIVO (fprintf)
     no arquivo "saida_<nome-da-entrada>.txt", com um resumo final:
     total de tokens validos e total de erros lexicos.
     O programa devolve exit code 1 se houve erro lexico.
   ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    S0,   /* estado INICIAL: ainda nao comecou nenhum token            */
    S1,   /* estado de leitura de ID: ja leu 1+ letras (loop letter*)  */
    S2    /* estado de leitura de NUM: ja leu 1+ digitos (loop digit*) */
} Estado;

/* ----------------------------------------------------------------------------
   DEFINICAO DOS TIPOS DE TOKEN
   ----------------------------------------------------------------------------
   Todo token reconhecido recebe uma destas classificacoes:
     T_ID      : identificador (ER letter letter*)
     T_NUM     : numero inteiro (ER digit digit*)
     T_KEYWORD : palavra reservada (int | void)
     T_SYMBOL  : um dos simbolos ; [ ]
     T_ERRO    : caractere que nao pertence a linguagem do subconjunto
     T_EOF     : marcador especial de fim de arquivo                     */
typedef enum {
    T_ID, T_NUM, T_KEYWORD, T_SYMBOL, T_ERRO, T_EOF
} TipoToken;
const char *nome_tipo[] = { "ID", "NUM", "KEYWORD", "SYMBOL", "ERRO", "EOF" };

/* ----------------------------------------------------------------------------
   ESTRUTURA DO TOKEN
   ----------------------------------------------------------------------------
   Cada token carrega:
     - tipo   : a classificacao (acima)
     - lexema : a string que foi reconhecida no fonte (ex.: "vetor", "10", ";")
     - linha  : numero da linha onde o token comecou (p mensagens de erro) */
typedef struct {
    TipoToken tipo;
    char      lexema[256];
    int       linha;
} Token;

/* ----------------------------------------------------------------------------
   TABELA DE PALAVRAS RESERVADAS DO SUBCONJUNTO
   ----------------------------------------------------------------------------
   "int" e "void" casam com a ER de ID (só letras). A diferenca entre KEYWORD
   e identificador comum só existe no FINAL do token: quando o DFA aceita em
   S1, comparamos o lexema com esta tabela com strcmp. O NULL final marca o
   fim da tabela.                                                               */
static const char *keywords[] = { "int", "void", NULL };

/* ----------------------------------------------------------------------------
   eh_simbolo(): verifica se o caractere eh um dos SIMBOLOS do subconjunto
   ----------------------------------------------------------------------------
   Os simbolos sao tokens de 1 unico caractere: ; [ e ].
   Como sao formados por apenas um caractere, o DFA vai de S0 direto para um
   estado de ACEITACAO imediato: nao ha estado intermediario necessario.       */
static int eh_simbolo(int c)
{
    return c == ';' || c == '[' || c == ']';
}

/* ============================================================================
   proximo_token(): PARTE PRINCIPAL DO SCANNER
   ============================================================================
   Executa o DFA minimo sobre o arquivo-fonte e devolve UM token por chamada
   (ou um T_ERRO, ou T_EOF quando o arquivo acabar).

   Estrategia geral (identica a de qualquer scanner):
     1. No estado S0, pular espacos/tabs e contar linhas (\n);
     2. Ver a PRIMEIRA letra do token para decidir a transicao de S0:
          letra      -> vai para S1 (vai ler um ID)
          digito     -> vai para S2 (vai ler um NUM)
          ;  [  ]    -> aceitacao IMEDIATA de SYMBOL
          outro      -> nenhum estado destino: ERRO LEXICO
     3. Em S1/S2, continuar lendo enquanto o caractere casar (auto-laco
        do * na ER: letter* / digit*);
     4. Quando o caractere NAO casar mais, o token acabou: ACEITAR o lexema
        acumulado e DEVOLVER esse caractere de terminacao com ungetc(),
        para que ele seja reprocessado no proximo token (nao pode ser
        descartado - ele pode ser o comeco do token seguinte, ex.:
        em "x;" o ';' encerra o ID "x" e ja' e' o comeco do SYMBOL ";").

   Parametros:
     f      : arquivo de entrada aberto (de onde os caracteres sao lidos)
     *linha : ponteiro para o contador de linhas compartilhado (atualizado
              a cada \n encontrado entre os tokens)
   Retorno: a estrutura Token preenchida.                                     */
Token proximo_token(FILE *f, int *linha)
{
    Token t = { T_EOF, "", *linha };   /* comeca assumindo fim de arquivo;  */
                                       /* se sair do loop abaixo, completa  */
    Estado e = S0;                     /* o DFA SEMPRE comeca no estado S0  */
    int c;                             /* caractere corrente lido           */
    int i = 0;                         /* indice no buffer do lexema         */
    int k;                             /* indice da tabela de keywords       */

    /* ------------------------------------------------------------
       PASSO 1 - ESTADO S0: pular espacos em branco e contar linhas
       ------------------------------------------------------------
       Espacos, tabs e \r NAO sao token: existem apenas para separar
       tokens. O \n tambem separa tokens, mas ainda precisamos conta-lo
       para saber em que linha cada token/erro ocorre (requisito para
       mensagens de erro uteis). Este loop so' termina quando encontrar
       um caractere que PODE iniciar um token - ou o fim do arquivo. */
    for (;;) {
        c = fgetc(f);
        if (c == EOF) return t;
        if (c == '\n') { (*linha)++; continue; } /* conta a linha e continua */
        if (c == ' ' || c == '\t' || c == '\r') continue; /* branco: ignora */
        break;                                   /* caractere util: inicia token */
    }

    t.linha = *linha;

    /* ------------------------------------------------------------
       PASSO 2 - TRANSICAO DE S0 PARA SIMBOLO (aceitacao imediata)
       ------------------------------------------------------------
       Se o caractere for ; ou [ ou ], ele sozinho ja' e' um token
       completo: nao ha estado intermediario. Aceita e retorna direto. */
    if (eh_simbolo(c)) {
        t.tipo      = T_SYMBOL;            /* classificacao do token        */
        t.lexema[0] = (char)c;             /* o lexema e' o proprio caractere */
        t.lexema[1] = '\0';                /* fecha a string do lexema      */
        return t;                          /* aceitacao imediata            */
    }

    /* ------------------------------------------------------------
       PASSO 3 - TRANSICAO S0 --letter--> S1 (leitura de um ID)
       ------------------------------------------------------------
       Primeiro caractere e' letra: muda para o estado S1 e entra no
       AUTO-LACO do * da ER "letter letter*": fica em S1 lendo letras
       enquanto houver letras. No primeiro caractere que NAO for letra,
       o loop termina: o token acabou.                                        */
    if (isalpha(c)) {                      /* isalpha == ER [a-zA-Z]        */
        e = S1;                            /* transicao S0 -> S1             */
        t.lexema[i++] = (char)c;           /* guarda a 1a letra do lexema   */

        /* AUTO-LACO do S1: enquanto vier letra, acumula no lexema.
           (cada letra lida consome a transicao S1 --letter--> S1)     */
        while ((c = fgetc(f)) != EOF && isalpha(c)) {
            if (i < 255) t.lexema[i++] = (char)c;   /* protege o buffer      */
        }
        t.lexema[i] = '\0';                /* fecha a string                 */

        /* O caractere atual encerrou o token. Se nao for EOF, DEVOLVE-O
           ao fluxo de entrada com ungetc() para ser reprocessado no
           proximo token (ex.: em "int x" o espaco termina "int", e em
           "10]" o ']' termina o NUM e ja' vira o proximo token).      */
        if (c != EOF) ungetc(c, f);

        /* Como "int" e "void" tambem casam com a ER de ID, o token ainda
           e' aceito como T_ID por enquanto...                            */
        t.tipo = T_ID;

        /* ...e SO AGORA consultamos a tabela de palavras reservadas:
           se o lexema for exatamente "int" ou "void", reclassificamos
           como T_KEYWORD. Isso equivale aos estados de aceitacao
           distintos que o DFA minimo criou para keywords.               */
        for (k = 0; keywords[k] != NULL; k++) {
            if (strcmp(t.lexema, keywords[k]) == 0) {
                t.tipo = T_KEYWORD;
                break;                      /* achou: nao precisa continuar   */
            }
        }
        return t;                          /* token ID/KEYWORD completo      */
    }

    /* ------------------------------------------------------------
       PASSO 4 - TRANSICAO S0 --digit--> S2 (leitura de um NUM)
       ------------------------------------------------------------
       Estrutura identica ao caso do ID, mas com digitos e a ER
       "digit digit*": primeiro caractere digito leva ao estado S2,
       e o auto-laco S2 --digit--> S2 acumula os demais digitos.               */
    if (isdigit(c)) {                      /* isdigit == ER [0-9]            */
        e = S2;                            /* transicao S0 -> S2             */
        t.lexema[i++] = (char)c;           /* guarda o 1o digito             */

        /* AUTO-LACO do S2: enquanto vier digito, acumula no lexema. */
        while ((c = fgetc(f)) != EOF && isdigit(c)) {
            if (i < 255) t.lexema[i++] = (char)c;
        }
        t.lexema[i] = '\0';

        /* Mesma tecnica do ID: devolve o terminador para reprocessar. */
        if (c != EOF) ungetc(c, f);

        t.tipo = T_NUM;                    /* aceitou "digit digit*"         */
        return t;
    }

    /* ------------------------------------------------------------
       PASSO 5 - NENHUMA TRANSICAO DEFINIDA: ERRO LEXICO
       ------------------------------------------------------------
       Chegamos aqui se o caractere nao e' letra, nem digito, nem um
       dos simbolos ; [ ]. No DFA isso significa um estado de ERRO
       (o caractere nao pertence a nenhuma ER do subconjunto).
       O scanner NAO para: reporta o erro e continua a analise a
       partir do proximo token (comportamento padrao de compiladores,
       para listar todos os erros de uma vez so).                            */
    t.tipo      = T_ERRO;
    t.lexema[0] = (char)c;                 /* o lexema do erro e' o caractere */
    t.lexema[1] = '\0';
    return t;
}

/* ============================================================================
   main(): PROGRAMA PRINCIPAL - driver do scanner
   ============================================================================
   Fluxo:
     1. Recebe o nome do arquivo-fonte pela linha de comando;
     2. Abre a ENTRADA (argv[1]) para leitura;
     3. Abre a SAIDA (saida_<arquivo>.txt) para gravar o log;
     4. Chama proximo_token() em LOOP ate receber T_EOF;
     5. Cada token/erro e' impresso NA TELA (printf) e SALVO NO ARQUIVO
        (fprintf) - requisito "guardar a saida em arquivo";
     6. Imprime o resumo (totais) nos dois destinos e fecha tudo.            */
int main(int argc, char *argv[])
{
    FILE *arq, *saida;                    /* arquivos de entrada e log        */
    char  nome_saida[512];                /* nome gerado do arquivo de log   */
    Token t;                              /* token corrente                   */
    int   linha = 1;                      /* contador de linhas (comeca em 1) */
    int   n_tokens = 0;                   /* total de tokens validos          */
    int   n_erros = 0;                    /* total de erros lexicos          */

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

    /* ---- LOOP PRINCIPAL: pede tokens ate o fim do arquivo ----
       Cada chamada a proximo_token() roda o DFA do inicio de um token
       ate a sua aceitacao (ou ate achar um caractere invalido).       */
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
            /* ---- Token de ERRO: reporta e conta ---- */
            n_erros++;
            printf("Linha %3d | %-7s | caractere invalido: '%s'\n",
                   t.linha, nome_tipo[T_ERRO], t.lexema);
            fprintf(saida, "Linha %3d | %-7s | caractere invalido: '%s'\n",
                    t.linha, nome_tipo[T_ERRO], t.lexema);
        } else {
            /* ---- Token valido: imprime linha, tipo e lexema ---- */
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
