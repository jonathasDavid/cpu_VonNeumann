# Simulador de CPU de Von Neumann

Trabalho de Arquitetura e Organizacao de Computadores I implementado em C.

O simulador carrega um arquivo texto com instrucoes e dados, monta as instrucoes no formato binario especificado no PDF e executa o ciclo de maquina:

1. Busca
2. Decodificacao
3. Execucao
4. Impressao dos registradores e da memoria
5. Pausa para o proximo ciclo, no modo interativo

## Requisitos implementados

- Memoria principal com 256 posicoes de 8 bits:

```c
unsigned char memoria[256];
```

- Barramento de dados de 8 bits: leituras e escritas em memoria passam byte a byte pelo MBR.
- Dados de 16 bits armazenados em dois bytes, byte mais significativo primeiro.
- Registradores arquiteturais:
  - `MBR`: 32 bits
  - `MAR`, `PC`, `IMM`: 16 bits
  - `IR`, `RO0`, `RO1`, `E`, `L`, `G`: 8 bits
  - `R0` a `R7`: vetor de registradores de 16 bits
- Instrucoes de 1, 2 ou 3 bytes, conforme o PDF.
- ISA completa do enunciado:
  - `hlt`, `nop`
  - `ldr`, `str`
  - `add`, `sub`, `mul`, `div`
  - `cmp`, `movr`
  - `and`, `or`, `xor`, `not`
  - `je`, `jne`, `jl`, `jle`, `jg`, `jge`, `jmp`
  - `ld`, `st`
  - `movi`, `addi`, `subi`, `muli`, `divi`
  - `lsh`, `rsh`

## Formato do arquivo de entrada

Cada linha deve seguir o formato:

```text
endereco;tipo;conteudo
```

Onde:

- `endereco`: hexadecimal.
- `tipo`: `i` para instrucao ou `d` para dado.
- `conteudo`: instrucao assembly ou valor hexadecimal de 16 bits.

Exemplo:

```text
0;i;ld r0, 90
3;i;ld r1, 92
6;i;add r0, r1
8;i;st r0, 94
b;i;hlt
90;d;5
92;d;3
94;d;0
```

Comentarios iniciados por `#` e linhas vazias sao ignorados.

## Compilacao

No Ubuntu, usando GCC:

```bash
gcc -std=c99 -Wall -Wextra -pedantic main.c -o cpu
```

No Windows com MinGW, o mesmo comando gera `cpu.exe`.

## Execucao

Modo interativo, conforme o PDF:

```bash
./cpu programa2.txt
```

Modo automatico para testes:

```bash
./cpu programa2.txt --batch
```

Tambem e possivel limitar ciclos para detectar loop infinito:

```bash
./cpu programa2.txt --batch --max-cycles 500
```

## Programas entregues

- `programa2.txt`: exercicio 2, produto interno entre dois vetores de 10 posicoes.
  - Vetor A: `0xD6` a `0xE9`
  - Vetor B: `0xEA` a `0xFD`
  - Resultado esperado em `0xFE/0xFF`: `0x01B8`

- `programa5.txt`: exercicio 5, conta valores maiores que `0x20`.
  - Resultado esperado em `0xFE/0xFF`: `0x000A`
  - Observacao: o PDF pede 20 palavras de 16 bits a partir de `0xD8`, mas tambem reserva `0xFE/0xFF` para o resultado. Vinte palavras a partir de `0xD8` terminam em `0xFF`; por isso este arquivo usa `0xFE/0xFF` como a ultima palavra inicial e sobrescreve esses bytes com o resultado final.

- `programa5_intervalo_pdf.txt`: variante literal do intervalo `0xD8` a `0xFD`.
  - Esse intervalo comporta 19 palavras de 16 bits.
  - Resultado esperado em `0xFE/0xFF`: `0x0009`

## Testes feitos

Comandos usados:

```bash
gcc -std=c99 -Wall -Wextra -pedantic main.c -o cpu
./cpu instrucoes.txt --batch --max-cycles 100
./cpu exemplo_pdf_formula.txt --batch --max-cycles 200
./cpu exemplo_pdf_somatorio.txt --batch --max-cycles 1000
./cpu programa2.txt --batch --max-cycles 500
./cpu programa5.txt --batch --max-cycles 500
./cpu programa5_intervalo_pdf.txt --batch --max-cycles 500
```

Resultados conferidos:

- `instrucoes.txt`: `0x94/0x95 = 0x0008`
- `exemplo_pdf_formula.txt`: `0x8E/0x8F = 0x0026`
- `exemplo_pdf_somatorio.txt`: `0x8A/0x8B = 0x0032`
- `programa2.txt`: `0xFE/0xFF = 0x01B8`
- `programa5.txt`: `0xFE/0xFF = 0x000A`
- `programa5_intervalo_pdf.txt`: `0xFE/0xFF = 0x0009`
