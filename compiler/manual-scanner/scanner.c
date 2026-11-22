#include "scanner.h"

#include <ctype.h>
#include <string.h>

typedef enum {
    S0, S1, S2
} Estado;

static const char *keywords[] = { "int", "void", NULL };

static int eh_simbolo(int c)
{
    return c == ';' || c == '[' || c == ']';
}

Token proximo_token(FILE *f, int *linha)
{
    Token t = { LEX_EOF, "", *linha };
    int c;
    int i = 0;
    int k;

    for (;;) {
        c = fgetc(f);
        if (c == EOF) return t;
        if (c == '\n') { (*linha)++; continue; }
        if (c == ' ' || c == '\t' || c == '\r') continue;
        break;
    }

    t.linha = *linha;

    if (eh_simbolo(c)) {
        t.tipo      = LEX_SYMBOL;
        t.lexema[0] = (char)c;
        t.lexema[1] = '\0';
        return t;
    }

    if (isalpha(c)) {
        t.lexema[i++] = (char)c;

        while ((c = fgetc(f)) != EOF && isalpha(c)) {
            if (i < 255) t.lexema[i++] = (char)c;
        }
        t.lexema[i] = '\0';

        if (c != EOF) ungetc(c, f);

        t.tipo = LEX_ID;
        for (k = 0; keywords[k] != NULL; k++) {
            if (strcmp(t.lexema, keywords[k]) == 0) {
                t.tipo = LEX_KEYWORD;
                break;
            }
        }
        return t;
    }

    if (isdigit(c)) {
        t.lexema[i++] = (char)c;

        while ((c = fgetc(f)) != EOF && isdigit(c)) {
            if (i < 255) t.lexema[i++] = (char)c;
        }
        t.lexema[i] = '\0';

        if (c != EOF) ungetc(c, f);

        t.tipo = LEX_NUM;
        return t;
    }

    t.tipo      = LEX_ERRO;
    t.lexema[0] = (char)c;
    t.lexema[1] = '\0';
    return t;
}
