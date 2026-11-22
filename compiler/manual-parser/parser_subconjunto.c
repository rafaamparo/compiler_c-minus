#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../manual-scanner/scanner.h"

/* =========================================================================
   DEFINIÇÕES DO ANALISADOR SINTÁTICO LL(1)
   ========================================================================= */

typedef enum {
    /* Terminais (0 a 7) */
    T_INT = 0,
    T_VOID,
    T_ID,
    T_NUM,
    T_SEMI,     // ;
    T_LBRACK,   // [
    T_RBRACK,   // ]
    T_EOF,      // $

    /* Não-Terminais (8 a 13) */
    NT_PROGRAM,
    NT_DECL_LIST,
    NT_DECL_LIST_PRIME,
    NT_DECL,
    NT_TYPE,
    NT_DECL_TAIL,

    /* Especiais */
    SYM_EPSILON,
    SYM_NONE
} Symbol;

#define IS_TERMINAL(s) ((s) <= T_EOF)
#define NUM_TERMINALS 8
#define NUM_NON_TERMINALS 6

const char* sym_names[] = {
    "int", "void", "ID", "NUM", ";", "[", "]", "$",
    "Program", "DeclList", "DeclList'", "Decl", "Type", "DeclTail",
    "epsilon", "none"
};

Symbol mapear_token(Token t) {
    if (t.tipo == LEX_EOF) return T_EOF;
    if (t.tipo == LEX_ID)  return T_ID;
    if (t.tipo == LEX_NUM) return T_NUM;

    if (t.tipo == LEX_KEYWORD) {
        if (strcmp(t.lexema, "int") == 0)  return T_INT;
        if (strcmp(t.lexema, "void") == 0) return T_VOID;
    }

    if (t.tipo == LEX_SYMBOL) {
        if (t.lexema[0] == ';') return T_SEMI;
        if (t.lexema[0] == '[') return T_LBRACK;
        if (t.lexema[0] == ']') return T_RBRACK;
    }

    if (t.tipo == LEX_ERRO) {
        printf("Erro Lexico na linha %d: Caractere invalido '%s'\n", t.linha, t.lexema);
        exit(1);
    }

    return SYM_NONE;
}

/* --- Estrutura da Árvore Sintática --- */
typedef struct Node {
    Symbol symbol;
    char lexeme[256];
    struct Node* children[5];
    int num_children;
} Node;

Node* create_node(Symbol sym, const char* lexeme) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->symbol = sym;
    if (lexeme) {
        strncpy(node->lexeme, lexeme, 255);
        node->lexeme[255] = '\0';
    } else {
        node->lexeme[0] = '\0';
    }
    node->num_children = 0;
    return node;
}

void add_child(Node* parent, Node* child) {
    parent->children[parent->num_children++] = child;
}

/* --- Tabela LL(1) ---
   Regras mapeadas (1 a 9). 0 = Erro Sintático.
   Colunas: int, void, ID, NUM, ;, [, ], $
*/
int table[NUM_NON_TERMINALS][NUM_TERMINALS] = {
    /*                int  void  ID   NUM   ;    [    ]    $  */
    /* Program   */ {  1,   1,    0,   0,   0,   0,   0,   0 },
    /* DeclList  */ {  2,   2,    0,   0,   0,   0,   0,   0 },
    /* DeclList' */ {  3,   3,    0,   0,   0,   0,   0,   4 },
    /* Decl      */ {  5,   5,    0,   0,   0,   0,   0,   0 },
    /* Type      */ {  6,   7,    0,   0,   0,   0,   0,   0 },
    /* DeclTail  */ {  0,   0,    0,   0,   8,   9,   0,   0 }
};

/* --- Pilha Sintática --- */
typedef struct {
    Symbol symbol;
    Node* tree_node;
} StackItem;

StackItem stack[256];
int top = -1;

void push(Symbol sym, Node* node) {
    stack[++top] = (StackItem){sym, node};
}

StackItem pop() {
    return stack[top--];
}

void print_stack() {
    printf("Pilha: ");
    for (int i = 0; i <= top; i++) {
        printf("%s ", sym_names[stack[i].symbol]);
    }
    printf("\n");
}

void print_tree(Node* root, int* is_last, int level) {
    if (!root) return;

    for (int i = 0; i < level; i++) {
        if (i == level - 1) {
            printf(is_last[i] ? "└── " : "├── ");
        } else {
            printf(is_last[i] ? "    " : "│   ");
        }
    }

    printf("%s", sym_names[root->symbol]);

    if ((root->symbol == T_NUM || root->symbol == T_ID) && root->lexeme[0] != '\0') {
        printf(" (%s)", root->lexeme);
    } else if (root->symbol == SYM_EPSILON) {
        printf(" (ε)");
    }
    printf("\n");

    for (int i = 0; i < root->num_children; i++) {
        is_last[level] = (i == root->num_children - 1);
        print_tree(root->children[i], is_last, level + 1);
    }
}

/* =========================================================================
   PARTE 3: FLUXO PRINCIPAL
   ========================================================================= */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte.cm>\n", argv[0]);
        return 1;
    }

    FILE *arq = fopen(argv[1], "r");
    if (arq == NULL) {
        perror(argv[1]);
        return 1;
    }

    int linha = 1;

    Node* root = create_node(NT_PROGRAM, NULL);
    push(T_EOF, NULL);
    push(NT_PROGRAM, root);

    Token lookahead = proximo_token(arq, &linha);
    Symbol lookahead_sym = mapear_token(lookahead);

    printf("\n--- Inicio do Parse (%s) ---\n\n", argv[1]);

    while (top >= 0) {
        print_stack();
        printf("Entrada (linha %d): '%s' [%s]\n\n",
               lookahead.linha, lookahead.lexema, sym_names[lookahead_sym]);

        StackItem current = pop();
        Symbol X = current.symbol;
        Node* current_node = current.tree_node;

        if (IS_TERMINAL(X)) {
            if (X == lookahead_sym) {
                if (current_node) {
                    strncpy(current_node->lexeme, lookahead.lexema, 255);
                    current_node->lexeme[255] = '\0';
                }
                if (X != T_EOF) {
                    lookahead = proximo_token(arq, &linha);
                    lookahead_sym = mapear_token(lookahead);
                }
            } else {
                printf("Erro Sintatico na linha %d: Esperado '%s', encontrado '%s'\n",
                       lookahead.linha, sym_names[X], lookahead.lexema);
                fclose(arq);
                return 1;
            }
        } else {
            int row = X - NT_PROGRAM;
            int col = lookahead_sym;
            int rule = table[row][col];

            if (rule == 0) {
                printf("Erro Sintatico na linha %d: Token inesperado '%s'\n",
                       lookahead.linha, lookahead.lexema);
                fclose(arq);
                return 1;
            }

            printf(">>> Aplicando Regra %d\n", rule);

            Symbol push_syms[5];
            int n_syms = 0;

            switch (rule) {
                case 1: /* Program -> DeclList */
                    push_syms[0] = NT_DECL_LIST; n_syms = 1; break;
                case 2: /* DeclList -> Decl DeclList' */
                    push_syms[0] = NT_DECL; push_syms[1] = NT_DECL_LIST_PRIME; n_syms = 2; break;
                case 3: /* DeclList' -> Decl DeclList' */
                    push_syms[0] = NT_DECL; push_syms[1] = NT_DECL_LIST_PRIME; n_syms = 2; break;
                case 4: /* DeclList' -> epsilon */
                    push_syms[0] = SYM_EPSILON; n_syms = 1; break;
                case 5: /* Decl -> Type ID DeclTail */
                    push_syms[0] = NT_TYPE; push_syms[1] = T_ID; push_syms[2] = NT_DECL_TAIL; n_syms = 3; break;
                case 6: /* Type -> int */
                    push_syms[0] = T_INT; n_syms = 1; break;
                case 7: /* Type -> void */
                    push_syms[0] = T_VOID; n_syms = 1; break;
                case 8: /* DeclTail -> ; */
                    push_syms[0] = T_SEMI; n_syms = 1; break;
                case 9: /* DeclTail -> [ NUM ] ; */
                    push_syms[0] = T_LBRACK; push_syms[1] = T_NUM; push_syms[2] = T_RBRACK; push_syms[3] = T_SEMI; n_syms = 4; break;
            }

            Node* child_nodes[5];
            for (int i = 0; i < n_syms; i++) {
                child_nodes[i] = create_node(push_syms[i], NULL);
                add_child(current_node, child_nodes[i]);
            }

            for (int i = n_syms - 1; i >= 0; i--) {
                if (push_syms[i] != SYM_EPSILON) {
                    push(push_syms[i], child_nodes[i]);
                }
            }
        }
    }

    printf("\n--- Codigo Aceito! ---\n\n");
    printf("Arvore Sintatica Resultante:\n");

    int is_last[100] = {0};
    print_tree(root, is_last, 0);

    fclose(arq);
    return 0;
}