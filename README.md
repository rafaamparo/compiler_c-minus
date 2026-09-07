# Projeto de Compilador: C--

## Sobre o Projeto

Este repositório contém a implementação de um compilador para uma versão simplificada da linguagem **C--**. O objetivo final do projeto é realizar desde a análise léxica até a geração de código em MIPS, que poderá ser executado em um simulador (como o MARS ou SPIM).

**Integrantes:**

- Luiz Paulo Sousa de Andrade
- Melinda Bianco Blak
- Rafael Vinícius Andrade Amparo

---

## Etapas do Compilador

O projeto está dividido nas seguintes fases:

- [x] **Etapa 1:** Analisador Léxico (Scanner)
- [ ] **Etapa 2:** Analisador Sintático e Semântico
- [ ] **Etapa 3:** Tradução para código assembly MIPS
- [ ] **Etapa 4:** Execução em simulador MIPS

---

## Etapa 1: Analisador Léxico (Fase Atual)

A primeira etapa foca no reconhecimento de tokens da linguagem, dividida em duas abordagens: uma manual (para fins didáticos) e uma automatizada utilizando o Flex.

A especificação sintática (BNF) completa da linguagem pode ser encontrada no arquivo `cminus-bnf.pdf`.
