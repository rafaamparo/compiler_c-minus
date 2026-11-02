#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Definições de Símbolos ---
typedef enum {
    T_PLUS = 0, T_MINUS, T_MUL, T_DIV, T_POW,
    T_LPAREN, T_RPAREN, T_NUM, T_EOF,
    NT_E, NT_E_PRIME, NT_T, NT_T_PRIME, NT_F, NT_F_PRIME, NT_P,
    SYM_EPSILON, SYM_NONE
} Symbol;

#define IS_TERMINAL(s) ((s) <= T_EOF)

const char* sym_names[] = {
    "+", "-", "*", "/", "^", "(", ")", "num", "$",
    "E", "E'", "T", "T'", "F", "F'", "P", "epsilon", "none"
};

// --- Estrutura da Árvore ---
typedef struct Node {
    Symbol symbol;
    char lexeme[32];
    struct Node* children[5];
    int num_children;
} Node;

Node* create_node(Symbol sym, const char* lexeme) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->symbol = sym;
    if (lexeme) strcpy(node->lexeme, lexeme);
    else node->lexeme[0] = '\0';
    node->num_children = 0;
    return node;
}

void add_child(Node* parent, Node* child) {
    parent->children[parent->num_children++] = child;
}

// --- Analisador Léxico ---
typedef struct {
    Symbol type;
    char lexeme[32];
} Token;

const char* input_str;
int pos = 0;

Token get_next_token() {
    Token token;
    while (isspace(input_str[pos])) pos++;

    char c = input_str[pos];
    if (c == '\0') { token.type = T_EOF; strcpy(token.lexeme, "$"); return token; }

    if (isdigit(c) || c == '.') {
        int i = 0;
        while (isdigit(input_str[pos]) || input_str[pos] == '.') {
            token.lexeme[i++] = input_str[pos++];
        }
        token.lexeme[i] = '\0';
        token.type = T_NUM;
        return token;
    }

    token.lexeme[0] = c;
    token.lexeme[1] = '\0';
    pos++;

    switch (c) {
        case '+': token.type = T_PLUS; break;
        case '-': token.type = T_MINUS; break;
        case '*': token.type = T_MUL; break;
        case '/': token.type = T_DIV; break;
        case '^': token.type = T_POW; break;
        case '(': token.type = T_LPAREN; break;
        case ')': token.type = T_RPAREN; break;
        default:
            printf("Erro Lexico: Caractere invalido '%c'\n", c);
            exit(1);
    }
    return token;
}

// --- Tabela LL(1) ---
// Regras mapeadas (1 a 13). 0 = Erro.
int table[7][9] = {
    // +, -, *, /, ^, (, ), num, $
    /* E  */ {0, 0, 0, 0, 0, 1, 0, 1, 0},
    /* E' */ {2, 3, 0, 0, 0, 0, 4, 0, 4},
    /* T  */ {0, 0, 0, 0, 0, 5, 0, 5, 0},
    /* T' */ {8, 8, 6, 7, 0, 0, 8, 0, 8},
    /* F  */ {0, 0, 0, 0, 0, 9, 0, 9, 0},
    /* F' */ {11,11,11,11, 10,0, 11,0, 11},
    /* P  */ {0, 0, 0, 0, 0, 12,0, 13,0}
};

// --- Analisador Sintático e Manipulação de Pilha ---
typedef struct {
    Symbol symbol;
    Node* tree_node;
} StackItem;

StackItem stack[100];
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

    // Imprime os prefixos de formatação para o nível atual
    for (int i = 0; i < level; i++) {
        if (i == level - 1) {
            printf(is_last[i] ? "└── " : "├── ");
        } else {
            printf(is_last[i] ? "    " : "│   ");
        }
    }

    // Imprime o símbolo do nó
    printf("%s", sym_names[root->symbol]);

    // Adiciona o lexema (valor) se for um número, ou o símbolo epsilon
    if (root->symbol == T_NUM && root->lexeme[0] != '\0') {
        printf(" (%s)", root->lexeme);
    } else if (root->symbol == SYM_EPSILON) {
        printf(" (ε)");
    }
    printf("\n");

    // Chamada recursiva para os filhos
    for (int i = 0; i < root->num_children; i++) {
        is_last[level] = (i == root->num_children - 1);
        print_tree(root->children[i], is_last, level + 1);
    }
}

int main() {
    char input_buffer[256];
    printf("Digite a expressao matematica (ex: 3.5 + 4 * ( 2 - 1 )): \n");
    if (!fgets(input_buffer, sizeof(input_buffer), stdin)) return 0;
    input_str = input_buffer;

    // Inicializa Pilha e Raiz
    Node* root = create_node(NT_E, NULL);
    push(T_EOF, NULL);
    push(NT_E, root);

    Token lookahead = get_next_token();
    printf("\n--- Inicio do Parse ---\n");

    while (top >= 0) {
        print_stack();
        printf("Entrada (lookahead): %s\n\n", lookahead.lexeme);

        StackItem current = pop();
        Symbol X = current.symbol;
        Node* current_node = current.tree_node;

        if (IS_TERMINAL(X)) {
            if (X == lookahead.type) {
                if (current_node) strcpy(current_node->lexeme, lookahead.lexeme);
                if (X != T_EOF) lookahead = get_next_token();
            } else {
                printf("Erro Sintatico: Esperado '%s', encontrado '%s'\n", sym_names[X], lookahead.lexeme);
                return 1;
            }
        } else {
            int row = X - NT_E;
            int col = lookahead.type;
            int rule = table[row][col];

            if (rule == 0) {
                printf("Erro Sintatico: Token inesperado '%s'\n", lookahead.lexeme);
                return 1;
            }

            printf(">>> Aplicando Regra %d\n", rule);

            // Variáveis auxiliares para empilhar filhos na árvore e na pilha sintática
            Symbol push_syms[5];
            int n_syms = 0;

            switch (rule) {
                case 1:  push_syms[0]=NT_T; push_syms[1]=NT_E_PRIME; n_syms=2; break; // E -> T E'
                case 2:  push_syms[0]=T_PLUS; push_syms[1]=NT_T; push_syms[2]=NT_E_PRIME; n_syms=3; break; // E' -> + T E'
                case 3:  push_syms[0]=T_MINUS; push_syms[1]=NT_T; push_syms[2]=NT_E_PRIME; n_syms=3; break; // E' -> - T E'
                case 4:  push_syms[0]=SYM_EPSILON; n_syms=1; break; // E' -> eps
                case 5:  push_syms[0]=NT_F; push_syms[1]=NT_T_PRIME; n_syms=2; break; // T -> F T'
                case 6:  push_syms[0]=T_MUL; push_syms[1]=NT_F; push_syms[2]=NT_T_PRIME; n_syms=3; break; // T' -> * F T'
                case 7:  push_syms[0]=T_DIV; push_syms[1]=NT_F; push_syms[2]=NT_T_PRIME; n_syms=3; break; // T' -> / F T'
                case 8:  push_syms[0]=SYM_EPSILON; n_syms=1; break; // T' -> eps
                case 9:  push_syms[0]=NT_P; push_syms[1]=NT_F_PRIME; n_syms=2; break; // F -> P F'
                case 10: push_syms[0]=T_POW; push_syms[1]=NT_F; n_syms=2; break; // F' -> ^ F
                case 11: push_syms[0]=SYM_EPSILON; n_syms=1; break; // F' -> eps
                case 12: push_syms[0]=T_LPAREN; push_syms[1]=NT_E; push_syms[2]=T_RPAREN; n_syms=3; break; // P -> ( E )
                case 13: push_syms[0]=T_NUM; n_syms=1; break; // P -> num
            }

            // Criação dos nós da árvore (ordem normal) e inserção na pilha (ordem inversa)
            Node* child_nodes[5];
            for (int i = 0; i < n_syms; i++) {
                child_nodes[i] = create_node(push_syms[i], NULL);
                add_child(current_node, child_nodes[i]);
            }

            // Empilha de trás para frente para manter a ordem LL(1)
            for (int i = n_syms - 1; i >= 0; i--) {
                if (push_syms[i] != SYM_EPSILON) {
                    push(push_syms[i], child_nodes[i]);
                }
            }
        }
    }

    printf("\n--- Expressao Aceita! ---\n\n");
    printf("Arvore Sintatica Resultante:\n");

    int is_last[100] = {0}; // Suporta árvores de até 100 níveis de profundidade
    print_tree(root, is_last, 0);

    return 0;
}