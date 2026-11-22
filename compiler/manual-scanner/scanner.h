#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>

typedef enum {
    LEX_ID, LEX_NUM, LEX_KEYWORD, LEX_SYMBOL, LEX_ERRO, LEX_EOF
} TipoToken;

typedef struct {
    TipoToken tipo;
    char      lexema[256];
    int       linha;
} Token;

Token proximo_token(FILE *f, int *linha);

#endif
