# Simulador de CPU - Arquitetura de von Neumann

Projeto desenvolvido para a disciplina de **Arquitetura e Organização de Computadores I**, com o objetivo de simular o funcionamento básico de uma CPU baseada no modelo da **Máquina de von Neumann**.

O simulador executa instruções em linguagem assembly simplificada, carregando um programa em memória e exibindo o estado da CPU ao final de cada ciclo de máquina.

## Objetivo do Projeto

O objetivo principal deste trabalho é compreender o funcionamento do **ciclo de instrução** de uma CPU de von Neumann, passando pelas etapas de:

1. Busca da instrução na memória;
2. Decodificação da instrução;
3. Execução da instrução;
4. Atualização dos registradores;
5. Exibição do estado da CPU após cada ciclo.

Conforme especificado no trabalho, a CPU simulada deve mostrar o conteúdo dos registradores e da memória ao final de cada ciclo de máquina, aguardando o usuário pressionar uma tecla para continuar a execução.

## Modelo de von Neumann

A arquitetura de von Neumann é baseada na ideia de que **instruções e dados ficam armazenados na mesma memória**.

Neste projeto, a CPU acessa a memória para buscar instruções e também para ler ou gravar dados. A execução ocorre de maneira sequencial, controlada principalmente pelo registrador **PC**, que aponta para a próxima instrução a ser executada.

## Características da CPU Simulada

A CPU simulada possui:

- Memória principal com 256 posições;
- Cada posição da memória possui 8 bits;
- Dados processados em palavras de 16 bits;
- Barramento de dados de 8 bits;
- Instruções de tamanho variável;
- Registradores arquiteturais;
- Unidade de controle;
- Unidade lógica e aritmética;
- Conjunto próprio de instruções, também chamado de ISA.

A memória deve ser implementada como um vetor de 256 posições do tipo `unsigned char`:

```c
unsigned char memoria[256];
```

Cada posição da memória representa um byte.

## Registradores

A CPU possui os seguintes registradores:

| Registrador | Função |
|------------|--------|
| PC | Program Counter. Guarda o endereço da próxima instrução |
| IR | Instruction Register. Guarda o opcode da instrução atual |
| MAR | Memory Address Register. Guarda o endereço de memória a ser acessado |
| MBR | Memory Buffer Register. Guarda o dado lido ou a ser escrito na memória |
| RO0 | Guarda o primeiro registrador operando |
| RO1 | Guarda o segundo registrador operando |
| IMM | Guarda valores imediatos ou endereços |
| E | Flag de igualdade |
| L | Flag de menor que |
| G | Flag de maior que |
| R0 a R7 | Registradores de propósito geral |

Os registradores de propósito geral são utilizados para armazenar temporariamente valores usados pela ALU durante operações aritméticas, lógicas e de movimentação de dados.

## Conjunto de Instruções

A CPU possui instruções como:

| Instrução | Descrição |
|----------|-----------|
| `hlt` | Finaliza a execução do programa |
| `nop` | Não realiza operação |
| `ld rX, Z` | Carrega no registrador `rX` o valor armazenado na memória |
| `st rX, Z` | Armazena na memória o valor de `rX` |
| `movi rX, IMM` | Move um valor imediato para `rX` |
| `add rX, rY` | Soma `rY` em `rX` |
| `sub rX, rY` | Subtrai `rY` de `rX` |
| `mul rX, rY` | Multiplica `rX` por `rY` |
| `div rX, rY` | Divide `rX` por `rY` |
| `cmp rX, rY` | Compara dois registradores e atualiza as flags |
| `jmp Z` | Salta para o endereço `Z` |
| `je Z` | Salta se a flag de igualdade estiver ativa |
| `jne Z` | Salta se a flag de igualdade não estiver ativa |
| `jl Z` | Salta se a flag de menor estiver ativa |
| `jle Z` | Salta se for menor ou igual |
| `jg Z` | Salta se a flag de maior estiver ativa |
| `jge Z` | Salta se for maior ou igual |

O conjunto completo de instruções inclui operações aritméticas, lógicas, movimentação de dados, comparação e desvios condicionais/incondicionais.

## Ciclo de Execução

O funcionamento básico do simulador segue a sequência:

```text
Início
  ↓
Carrega o programa na memória
  ↓
Busca a próxima instrução usando o PC
  ↓
Decodifica a instrução
  ↓
Executa a instrução
  ↓
Atualiza registradores e memória
  ↓
Exibe o estado da CPU
  ↓
Aguarda uma tecla para continuar
  ↓
Repete até encontrar HLT
```

O programa deve continuar executando até encontrar a instrução `hlt`, que representa o fim da execução.

## Formato do Arquivo de Entrada

O simulador deve ler um arquivo `.txt` contendo as instruções e os dados do programa.

Exemplo de formato:

```text
0;i;ld r0, 96
3;i;ld r1, 98
6;i;sub r0, r1
8;i;ld r1, 94
b;i;div r1, r0
d;i;ld r2, 92
10;i;mul r2, r1
12;i;ld r1, 90
15;i;add r1, r2
17;i;st r1, 8e
1a;i;hlt
90;d;20
92;d;3
94;d;4
96;d;5
98;d;3
```

Cada linha segue o formato:

```text
endereço;tipo;conteúdo
```

Onde:

- `endereço`: endereço inicial na memória, em hexadecimal;
- `tipo`: indica se é instrução ou dado;
  - `i` para instrução;
  - `d` para dado;
- `conteúdo`: instrução assembly ou valor numérico.

## Exemplo de Uso

Para executar o simulador, utilize um arquivo de entrada contendo o programa em assembly.

Exemplo:

```bash
./cpu programa.txt
```

Durante a execução, o simulador deverá mostrar o estado da CPU e da memória ao final de cada ciclo.

Exemplo de saída esperada:

```text
CPU:
R0: 0000 R1: 0000 R2: 0000 R3: 0000
R4: 0000 R5: 0000 R6: 0000 R7: 0000

MBR: 00000000 MAR: 0000 IMM: 0000 PC: 0000
IR: 00 RO0: 0 RO1: 0
E: 0 L: 0 G: 0

Memória:
00 A8 00 1E A9 00 20 20 20 C0 00 14 B0 00 22 00
...

Pressione uma tecla para iniciar o próximo ciclo de máquina ou CTRL+C para finalizar.
```

## Compilação

O projeto deve ser compilado utilizando o **GCC no Ubuntu**.

Comando de compilação:

```bash
gcc main.c -o cpu
```

Caso o projeto esteja separado em vários arquivos `.c`, utilize:

```bash
gcc main.c cpu.c memoria.c instrucoes.c -o cpu
```

## Execução

Após compilar, execute o programa passando o arquivo de entrada:

```bash
./cpu programa.txt
```

## Estrutura Sugerida do Projeto

```text
.
├── main.c
├── cpu.c
├── cpu.h
├── memoria.c
├── memoria.h
├── instrucoes.c
├── instrucoes.h
├── programa.txt
└── README.md
```

## Descrição dos Arquivos

| Arquivo | Descrição |
|--------|-----------|
| `main.c` | Arquivo principal do programa |
| `cpu.c` | Implementação das funções da CPU |
| `cpu.h` | Definições dos registradores e funções da CPU |
| `memoria.c` | Funções de leitura e escrita na memória |
| `memoria.h` | Definições relacionadas à memória |
| `instrucoes.c` | Implementação das instruções da ISA |
| `instrucoes.h` | Declaração das instruções |
| `programa.txt` | Arquivo contendo o programa em assembly |
| `README.md` | Documentação do projeto |

## Implementação da Memória

A memória possui 256 posições de 8 bits:

```c
unsigned char memoria[256];
```

Como os dados processados pela CPU possuem 16 bits, uma palavra de 16 bits ocupa duas posições consecutivas da memória.

Exemplo:

```text
Endereço 0x90: byte mais significativo
Endereço 0x91: byte menos significativo
```

## Implementação dos Registradores

Exemplo de declaração dos registradores:

```c
unsigned int mbr;
unsigned short int mar;
unsigned char ir;
unsigned char ro0;
unsigned char ro1;
unsigned short int imm;
unsigned short int pc;
unsigned char e, l, g;
unsigned short int reg[8];
```

## Observações Importantes

- O projeto deve ser implementado em linguagem C.
- Não deve ser implementado em C++.
- A memória deve ser um vetor de bytes.
- Os registradores devem ser exibidos em hexadecimal.
- O simulador deve mostrar o estado da CPU ao final de cada ciclo.
- O usuário deve pressionar uma tecla para avançar para o próximo ciclo.
- A execução deve parar ao encontrar a instrução `hlt`.

## Autor

Projeto desenvolvido por:

```text
Nome: Jônathas David e Heitor Barreto
Curso: Bacharelado em Ciência da Computação
Disciplina: Arquitetura e Organização de Computadores I
Instituição: Instituto Federal de Goiás - Campus Anápolis
Professor: Hugo Vinicius Leao e Silva 
```

## Referência

Trabalho prático da disciplina de Arquitetura e Organização de Computadores I, cujo objetivo é implementar um simulador de CPU baseado no ciclo de instrução da Máquina de von Neumann.
