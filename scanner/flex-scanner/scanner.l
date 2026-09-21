%{
/* ============================================================
   Como compilar e executar (Linux/WSL):
     flex scanner.l
     gcc lex.yy.c -o scanner
     ./scanner teste_valido.cm      (cria saida_teste_valido.cm.txt)
     ./scanner teste_invalido.cm    (cria saida_teste_invalido.cm.txt)


   ATENCAO: comentarios NAO sao permitidos dentro da secao de
   regras do Flex (%% ... %%), por isso toda a explicacao esta
   neste cabecalho:
     1. "/*"            -> guarda a linha atual em
                           inicio_comentario e muda para o estado
                           exclusivo COMENTARIO;
     2. KEYWORD antes de {ID} -> Flex empata pelo casamento mais
                           longo e desempata pela primeira regra;
     3. Simbolos multi-caractere (<= >= == !=) listados junto aos
                           simples: o longest match do Flex escolhe
                           o maior;
     4. estado COMENTARIO: ignora conteudo, conta \n, volta ao
                           INITIAL no '*/'; se chegar ao <<EOF>>
                           dentro dele, reporta erro apontando a
                           linha de abertura e termina a analise;
     5. caractere invalido -> regra "." chama registrar_erro()
                           e a analise CONTINUA (lista todos os
                           erros de uma vez);
     6. main(): abre entrada e saida, roda yylex(), imprime o
                           token EOF e o resumo nos dois destinos.
   ============================================================ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int  linha            = 1;   /* contador de linhas do arquivo-fonte  */
int  n_tokens         = 0;   /* total de tokens validos reconhecidos  */
int  n_erros          = 0;   /* total de erros lexicos encontrados    */
int  inicio_comentario = 0;  /* linha onde o ultimo '/*' foi aberto   */
FILE *saida;                 /* arquivo de log da analise             */


void registrar(const char *classe);
void registrar_erro(int linha_err, const char *msg, const char *detalhe);
void registrar_eof(void);
%}


%option noyywrap


/* ---- Definicoes regulares (equivalentes as ERs da parte 1) ---- */
LETTER  [A-Za-z]
DIGIT   [0-9]
ID      {LETTER}({LETTER}|{DIGIT})*
NUM     {DIGIT}+


%x COMENTARIO


%%


<INITIAL>{
"/*"        { inicio_comentario = linha; BEGIN(COMENTARIO); }


"int"|"void"|"if"|"else"|"while"|"return" { registrar("KEYWORD"); }


{ID}        { registrar("ID"); }
{NUM}       { registrar("NUM"); }


"<="|">="|"=="|"!="|"+"|"-"|"*"|"/"|"<"|">"|"="|";"|","|"("|")"|"["|"]"|"{"|"}"        { registrar("SYMBOL"); }


\n          { linha++; }
[ \t\r]+    { }


.           { registrar_erro(linha, "caractere invalido:", yytext); }
}


<COMENTARIO>{
<<EOF>>     { registrar_erro(inicio_comentario,
                           "comentario '/*' aberto nesta linha nao foi fechado",
                           "");
              yyterminate();
            }
"*/"        { BEGIN(INITIAL); }
\n          { linha++; }
.           { }
}


%%


/* ---------- helpers: escrevem no console E no arquivo de log ---------- */


void registrar(const char *classe)
{
    printf("Linha %3d | %-7s | %s\n", linha, classe, yytext);
    fprintf(saida, "Linha %3d | %-7s | %s\n", linha, classe, yytext);
    n_tokens++;
}


/* linha_err permite escolher QUAL linha exibir no erro:
   - erros comuns  -> a linha corrente (variavel linha)
   - comentario nao fechado -> a linha de ABERTURA (inicio_comentario) */
void registrar_erro(int linha_err, const char *msg, const char *detalhe)
{
    n_erros++;
    printf("Linha %3d | %-7s | %s %s\n", linha_err, "ERRO", msg, detalhe);
    fprintf(saida, "Linha %3d | %-7s | %s %s\n", linha_err, "ERRO", msg, detalhe);
}


/* Token de fim de arquivo: fecha a lista de tokens no formato que um
   parser espera. Nao entra na contagem de n_tokens (e' um marcador,
   nao um token do fonte). */
void registrar_eof(void)
{
    printf("Linha %3d | %-7s | %s\n", linha, "EOF", "fim do arquivo");
    fprintf(saida, "Linha %3d | %-7s | %s\n", linha, "EOF", "fim do arquivo");
}


/* ---------- abertura de arquivos ---------- */


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
    FILE *arquivo;


    snprintf(nome_saida, sizeof(nome_saida), "saida_%s.txt", arquivo_entrada);
    arquivo = fopen(nome_saida, "w");
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


    printf("=== Analise lexica de: %s ===\n\n", argv[1]);
    fprintf(saida, "=== Analise lexica de: %s ===\n\n", argv[1]);


    yylex();
    registrar_eof();


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
