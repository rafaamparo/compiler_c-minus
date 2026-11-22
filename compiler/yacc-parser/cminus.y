%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declarações externas do Flex */
extern int yylex();
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

void yyerror(const char *s);

/* Estrutura básica para a Árvore Sintática Abstrata (AST) */
typedef struct ASTNode {
    char *type;
    char *value;
    struct ASTNode *child1;
    struct ASTNode *child2;
    struct ASTNode *child3;
    struct ASTNode *sibling;
} ASTNode;

ASTNode *root = NULL;

ASTNode* newNode(char *type, char *value, ASTNode *c1, ASTNode *c2, ASTNode *c3) {
    ASTNode *node = (ASTNode*) malloc(sizeof(ASTNode));
    node->type = strdup(type);
    node->value = value ? strdup(value) : NULL;
    node->child1 = c1;
    node->child2 = c2;
    node->child3 = c3;
    node->sibling = NULL;
    return node;
}

ASTNode* addSibling(ASTNode *head, ASTNode *new_sib) {
    if (!head) return new_sib;
    ASTNode *temp = head;
    while (temp->sibling) temp = temp->sibling;
    temp->sibling = new_sib;
    return head;
}

void printAST(ASTNode *node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    printf("%s", node->type);
    if (node->value) printf(" (%s)", node->value);
    printf("\n");
    printAST(node->child1, indent + 1);
    printAST(node->child2, indent + 1);
    printAST(node->child3, indent + 1);
    printAST(node->sibling, indent);
}
%}

/* Tipos semânticos */
%union {
    char *str;
    struct ASTNode *node;
}

/* Tokens terminais */
%token <str> ID NUM
%token ELSE IF INT RETURN VOID WHILE
%token PLUS MINUS TIMES OVER
%token LT LE GT GE EQ NE
%token ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE

/* Tipos dos não-terminais */
%type <node> program declaration_list declaration var_declaration type_specifier
%type <node> fun_declaration params param_list param compound_stmt
%type <node> local_declarations statement_list statement expression_stmt
%type <node> selection_stmt iteration_stmt return_stmt expression var
%type <node> simple_expression relop additive_expression addop term mulop
%type <node> factor call args arg_list

/* Resolve o conflito clássico de Shift/Reduce do "Dangling Else" */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%start program

%%

program
    : declaration_list { root = $1; }
    ;

declaration_list
    : declaration_list declaration { $$ = addSibling($1, $2); }
    | declaration                  { $$ = $1; }
    ;

declaration
    : var_declaration { $$ = $1; }
    | fun_declaration { $$ = $1; }
    ;

var_declaration
    : type_specifier ID SEMI
        { $$ = newNode("VarDecl", $2, $1, NULL, NULL); }
    | type_specifier ID LBRACK NUM RBRACK SEMI
        { $$ = newNode("ArrayDecl", $2, $1, newNode("Size", $4, NULL, NULL, NULL), NULL); }
    ;

type_specifier
    : INT  { $$ = newNode("Type", "int", NULL, NULL, NULL); }
    | VOID { $$ = newNode("Type", "void", NULL, NULL, NULL); }
    ;

fun_declaration
    : type_specifier ID LPAREN params RPAREN compound_stmt
        { $$ = newNode("FunDecl", $2, $1, $4, $6); }
    ;

params
    : param_list { $$ = $1; }
    | VOID       { $$ = newNode("Params", "void", NULL, NULL, NULL); }
    ;

param_list
    : param_list COMMA param { $$ = addSibling($1, $3); }
    | param                  { $$ = $1; }
    ;

param
    : type_specifier ID
        { $$ = newNode("Param", $2, $1, NULL, NULL); }
    | type_specifier ID LBRACK RBRACK
        { $$ = newNode("ArrayParam", $2, $1, NULL, NULL); }
    ;

compound_stmt
    : LBRACE local_declarations statement_list RBRACE
        { $$ = newNode("CompoundStmt", NULL, $2, $3, NULL); }
    ;

local_declarations
    : local_declarations var_declaration { $$ = addSibling($1, $2); }
    | /* vazio */                        { $$ = NULL; }
    ;

statement_list
    : statement_list statement { $$ = addSibling($1, $2); }
    | /* vazio */              { $$ = NULL; }
    ;

statement
    : expression_stmt { $$ = $1; }
    | compound_stmt   { $$ = $1; }
    | selection_stmt  { $$ = $1; }
    | iteration_stmt  { $$ = $1; }
    | return_stmt     { $$ = $1; }
    ;

expression_stmt
    : expression SEMI { $$ = $1; }
    | SEMI            { $$ = newNode("EmptyStmt", NULL, NULL, NULL, NULL); }
    ;

selection_stmt
    : IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE
        { $$ = newNode("If", NULL, $3, $5, NULL); }
    | IF LPAREN expression RPAREN statement ELSE statement
        { $$ = newNode("IfElse", NULL, $3, $5, $7); }
    ;

iteration_stmt
    : WHILE LPAREN expression RPAREN statement
        { $$ = newNode("While", NULL, $3, $5, NULL); }
    ;

return_stmt
    : RETURN SEMI
        { $$ = newNode("Return", NULL, NULL, NULL, NULL); }
    | RETURN expression SEMI
        { $$ = newNode("Return", NULL, $2, NULL, NULL); }
    ;

expression
    : var ASSIGN expression { $$ = newNode("Assign", "=", $1, $3, NULL); }
    | simple_expression     { $$ = $1; }
    ;

var
    : ID
        { $$ = newNode("Var", $1, NULL, NULL, NULL); }
    | ID LBRACK expression RBRACK
        { $$ = newNode("ArrayVar", $1, $3, NULL, NULL); }
    ;

simple_expression
    : additive_expression relop additive_expression
        { $2->child1 = $1; $2->child2 = $3; $$ = $2; }
    | additive_expression
        { $$ = $1; }
    ;

relop
    : LE { $$ = newNode("RelOp", "<=", NULL, NULL, NULL); }
    | LT { $$ = newNode("RelOp", "<", NULL, NULL, NULL); }
    | GT { $$ = newNode("RelOp", ">", NULL, NULL, NULL); }
    | GE { $$ = newNode("RelOp", ">=", NULL, NULL, NULL); }
    | EQ { $$ = newNode("RelOp", "==", NULL, NULL, NULL); }
    | NE { $$ = newNode("RelOp", "!=", NULL, NULL, NULL); }
    ;

additive_expression
    : additive_expression addop term
        { $2->child1 = $1; $2->child2 = $3; $$ = $2; }
    | term
        { $$ = $1; }
    ;

addop
    : PLUS  { $$ = newNode("AddOp", "+", NULL, NULL, NULL); }
    | MINUS { $$ = newNode("AddOp", "-", NULL, NULL, NULL); }
    ;

term
    : term mulop factor
        { $2->child1 = $1; $2->child2 = $3; $$ = $2; }
    | factor
        { $$ = $1; }
    ;

mulop
    : TIMES { $$ = newNode("MulOp", "*", NULL, NULL, NULL); }
    | OVER  { $$ = newNode("MulOp", "/", NULL, NULL, NULL); }
    ;

factor
    : LPAREN expression RPAREN { $$ = $2; }
    | var                      { $$ = $1; }
    | call                     { $$ = $1; }
    | NUM                      { $$ = newNode("Num", $1, NULL, NULL, NULL); }
    ;

call
    : ID LPAREN args RPAREN
        { $$ = newNode("Call", $1, $3, NULL, NULL); }
    ;

args
    : arg_list    { $$ = $1; }
    | /* vazio */ { $$ = NULL; }
    ;

arg_list
    : arg_list COMMA expression { $$ = addSibling($1, $3); }
    | expression                { $$ = $1; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintático na linha %d próximo a '%s': %s\n", yylineno, yytext, s);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            perror("Erro ao abrir arquivo");
            return 1;
        }
    }

    if (yyparse() == 0) {
        printf("Análise sintática concluída com sucesso!\n\n--- AST ---\n");
        printAST(root, 0);
    }
    return 0;
}