# Compilador C--

Implementação experimental de analisadores para um subconjunto da linguagem
**C--**, desenvolvida em C. O repositório reúne implementações manuais e
implementações geradas com Flex e Yacc/Bison, com foco nas etapas léxica e
sintática.

## Estado atual

- [x] Analisador léxico manual para o subconjunto da primeira etapa
- [x] Analisador sintático manual LL(1) para declarações do subconjunto
- [x] Analisador léxico completo com Flex
- [x] Analisador sintático com Yacc/Bison e Flex, gerando uma AST
- [x] Analisador sintático LL(1) de expressões matemáticas
- [ ] Análise semântica
- [ ] Geração e execução de código MIPS

A gramática de referência está em [`cminus-bnf.pdf`](cminus-bnf.pdf).

## Estrutura do repositório

```text
compiler/
├── manual-scanner/
│   ├── scanner.c                # Implementação reutilizável do léxico manual
│   ├── scanner.h                # Interface compartilhada com o parser
│   ├── scanner_subconjunto.c    # Scanner manual do subconjunto
│   └── sub_*.cm                 # Entradas de teste
├── manual-parser/
│   ├── parser_subconjunto.c     # Parser LL(1) manual adicional
│   └── sub_*.cm               # Entradas de teste
├── flex-scanner/
│   ├── scanner.l                # Lexer da primeira entrega e fonte usada pelo Yacc
│   └── teste_*.cm             # Entradas de teste
└── yacc-parser/
    ├── cminus.y                 # Gramática Yacc/Bison e construção da AST
    └── sub_*.cm               # Entradas de teste
parser_operacoes/
└── analyzer.c                 # Parser LL(1) interativo de expressões
```

## Fluxo principal de leitura

O lexer implementado com Flex na primeira entrega é a única fonte léxica
integrada ao parser Yacc/Bison:

```text
arquivo-fonte .cm
       |
       v
flex-scanner/scanner.l
       |
       v
yylex() retorna um token por chamada
       |
       v
    cminus.y
       |
       v
parser Yacc/Bison
       |
       +--> código válido --> análise sintática concluída + AST
       |
       +--> código inválido -> erro léxico e/ou erro sintático
```

O arquivo [scanner.l](compiler/flex-scanner/scanner.l) pode ser executado como
scanner independente ou integrado ao Yacc. Sem `YACC_PARSER`, ele imprime os
tokens reconhecidos. Com `-DYACC_PARSER`, ele inclui `y.tab.h` e devolve os
tokens ao `cminus.y` por meio de `yylex()`.

Os executáveis e arquivos gerados (`scanner`, `cminus_parser`, `lex.yy.c`,
`y.tab.c` e `y.tab.h`) também podem estar presentes no repositório para
facilitar a execução dos exemplos.

## Pré-requisitos

Para recompilar os analisadores, use um ambiente C com GCC. O scanner Flex e o
parser Yacc/Bison também exigem:

- [Flex](https://github.com/westes/flex)
- [Bison](https://www.gnu.org/software/bison/)

Os comandos abaixo funcionam em Linux ou WSL. Em outros ambientes, ajuste a
forma de invocar os executáveis.

## Como compilar e executar

### Scanner manual

```bash
cd compiler/manual-scanner
gcc scanner.c scanner_subconjunto.c -o scanner
./scanner sub_valido.cm
./scanner sub_invalido.cm
```

O programa imprime os tokens e erros léxicos no terminal e salva um relatório
`saida_<arquivo-de-entrada>.txt`.

### Parser manual LL(1)

```bash
cd compiler/manual-parser
gcc ../manual-scanner/scanner.c parser_subconjunto.c -o parser_subconjunto
./parser_subconjunto sub_valido.cm
./parser_subconjunto sub_invalido.cm
```

O parser manual reutiliza `proximo_token` do scanner em
`manual-scanner/scanner.c`; a implementação léxica não é duplicada no parser.

Durante o reconhecimento, o parser exibe a pilha, as regras aplicadas e a
árvore sintática resultante.

### Scanner com Flex

```bash
cd compiler/flex-scanner
flex scanner.l
gcc lex.yy.c -o scanner
./scanner teste_valido.cm
./scanner teste_invalido.cm
```

O scanner reconhece palavras-chave, identificadores, números, símbolos e
comentários `/* ... */`, acumulando os erros léxicos encontrados.

### Parser com Yacc/Bison e Flex

Para regenerar os arquivos do parser:

```bash
cd compiler/yacc-parser
bison -y -d cminus.y
flex ../flex-scanner/scanner.l
gcc -DYACC_PARSER y.tab.c lex.yy.c -o cminus_parser
./cminus_parser sub_valido.cm
./cminus_parser sub_invalido.cm
```

Dependendo da versão do Bison, os arquivos podem ser gerados com os nomes
`cminus.tab.c` e `cminus.tab.h`. Nesse caso, ajuste os nomes no comando de
compilação ou use os arquivos gerados já presentes no diretório. Ao aceitar a
entrada, o parser imprime a AST.

### Analisador de expressões matemáticas

```bash
cd parser_operacoes
gcc analyzer.c -o analyzer
./analyzer
```

Digite uma expressão usando números, `+`, `-`, `*`, `/`, `^` e parênteses, por
exemplo:

```text
3.5 + 4 * ( 2 - 1 )
```

O analisador mostra as regras LL(1) aplicadas e a árvore sintática.

## Integrantes

- Luiz Paulo Sousa de Andrade
- Melinda Bianco Blak
- Rafael Vinícius Andrade Amparo
