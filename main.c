#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

// Variáveis Globais
Registrador cpu;
unsigned char memoria[TAMANHO_MEMORIA];
int halt = 0;
int tamanhoInstrucaoAtual = 0;

// Funções Auxiliares de Memória
unsigned char lerByte(unsigned short int endereco) {
    cpu.MAR = endereco;
    cpu.MBR = memoria[endereco];
    return cpu.MBR & MASK_BYTE;
}

void escreverByte(unsigned short int endereco, unsigned char valor) {
    cpu.MAR = endereco;
    cpu.MBR = valor;
    memoria[endereco] = cpu.MBR & MASK_BYTE;
}

unsigned short int lerPalavra(unsigned short int endereco) {
    unsigned char alto = lerByte(endereco);
    unsigned char baixo = lerByte(endereco + 1);
    cpu.MAR = endereco;
    cpu.MBR = (alto << 8) | baixo;
    return cpu.MBR & MASK_WORD;
}

void escreverPalavra(unsigned short int endereco, unsigned short int valor) {
    cpu.MAR = endereco;
    cpu.MBR = valor;
    escreverByte(endereco, (valor >> 8) & MASK_BYTE);
    escreverByte(endereco + 1, valor & MASK_BYTE);
    cpu.MAR = endereco;
    cpu.MBR = valor;
}

// Retorna o Opcode baseado no texto (Mnemônico)
int pegaOpcode(char *mnem) {
    if (strcmp(mnem, "hlt") == 0) return OP_HLT;
    if (strcmp(mnem, "nop") == 0) return OP_NOP;
    if (strcmp(mnem, "ldr") == 0) return OP_LDR;
    if (strcmp(mnem, "str") == 0) return OP_STR;
    if (strcmp(mnem, "add") == 0) return OP_ADD;
    if (strcmp(mnem, "sub") == 0) return OP_SUB;
    if (strcmp(mnem, "mul") == 0) return OP_MUL;
    if (strcmp(mnem, "div") == 0) return OP_DIV;
    if (strcmp(mnem, "cmp") == 0) return OP_CMP;
    if (strcmp(mnem, "movr") == 0) return OP_MOVR;
    if (strcmp(mnem, "and") == 0) return OP_AND;
    if (strcmp(mnem, "or") == 0) return OP_OR;
    if (strcmp(mnem, "xor") == 0) return OP_XOR;
    if (strcmp(mnem, "not") == 0) return OP_NOT;
    if (strcmp(mnem, "je") == 0) return OP_JE;
    if (strcmp(mnem, "jne") == 0) return OP_JNE;
    if (strcmp(mnem, "jl") == 0) return OP_JL;
    if (strcmp(mnem, "jle") == 0) return OP_JLE;
    if (strcmp(mnem, "jg") == 0) return OP_JG;
    if (strcmp(mnem, "jge") == 0) return OP_JGE;
    if (strcmp(mnem, "jmp") == 0) return OP_JMP;
    if (strcmp(mnem, "ld") == 0) return OP_LD;
    if (strcmp(mnem, "st") == 0) return OP_ST;
    if (strcmp(mnem, "movi") == 0) return OP_MOVI;
    if (strcmp(mnem, "addi") == 0) return OP_ADDI;
    if (strcmp(mnem, "subi") == 0) return OP_SUBI;
    if (strcmp(mnem, "muli") == 0) return OP_MULI;
    if (strcmp(mnem, "divi") == 0) return OP_DIVI;
    if (strcmp(mnem, "lsh") == 0) return OP_LSH;
    if (strcmp(mnem, "rsh") == 0) return OP_RSH;
    return -1;
}

// Carrega o arquivo txt para a memória
void carregarArquivo(const char *nomeArquivo) {
    FILE *arq = fopen(nomeArquivo, "r");
    if (arq == NULL) {
        printf("Erro: Nao foi possivel abrir o arquivo %s\n", nomeArquivo);
        exit(1);
    }

    char linha[100];
    int primeiraInstrucao = 1;

    while (fgets(linha, 100, arq) != NULL) {
        unsigned int endereco;
        char tipo;
        char conteudo[50];

        if (sscanf(linha, "%x;%c;%[^\n]", &endereco, &tipo, conteudo) == 3) {
            
            if (tipo == 'd' || tipo == 'D') {
                // Dado (2 bytes)
                unsigned int valorDado;
                sscanf(conteudo, "%x", &valorDado);
                memoria[endereco] = (valorDado >> 8) & MASK_BYTE;
                memoria[endereco + 1] = valorDado & MASK_BYTE;
            } 
            else if (tipo == 'i' || tipo == 'I') {
                if (primeiraInstrucao) {
                    cpu.PC = endereco;
                    primeiraInstrucao = 0;
                }

                char mnem[10];
                int r0 = 0, r1 = 0;
                unsigned int val = 0;
                
                sscanf(conteudo, "%s", mnem);
                int opcode = pegaOpcode(mnem);

                if (opcode == OP_HLT || opcode == OP_NOP) {
                    memoria[endereco] = (opcode << 3);
                } 
                else if (opcode == OP_NOT) {
                    sscanf(conteudo, "%*s r%d", &r0);
                    memoria[endereco] = (opcode << 3) | r0;
                } 
                else if (opcode >= OP_LDR && opcode <= OP_XOR) { 
                    sscanf(conteudo, "%*s r%d, r%d", &r0, &r1);
                    memoria[endereco] = (opcode << 3) | r0;
                    memoria[endereco + 1] = (r1 << 5);
                } 
                else if (opcode >= OP_JE && opcode <= OP_JMP) { 
                    sscanf(conteudo, "%*s %x", &val);
                    memoria[endereco] = (opcode << 3);
                    memoria[endereco + 1] = (val >> 8) & MASK_BYTE;
                    memoria[endereco + 2] = val & MASK_BYTE;
                } 
                else if (opcode >= OP_LD && opcode <= OP_RSH) { 
                    sscanf(conteudo, "%*s r%d, %x", &r0, &val);
                    memoria[endereco] = (opcode << 3) | r0;
                    memoria[endereco + 1] = (val >> 8) & MASK_BYTE;
                    memoria[endereco + 2] = val & MASK_BYTE;
                }
            }
        }
    }
    fclose(arq);
}

// Imprime a CPU no formato pedido
void imprimeEstado() {
    printf("\nCPU:\n");
    printf("R0: %04X R1: %04X R2: %04X R3: %04X\n", cpu.reg[0], cpu.reg[1], cpu.reg[2], cpu.reg[3]);
    printf("R4: %04X R5: %04X R6: %04X R7: %04X\n", cpu.reg[4], cpu.reg[5], cpu.reg[6], cpu.reg[7]);
    printf("MBR: %08X MAR: %04X IMM: %04X PC: %04X\n", cpu.MBR, cpu.MAR, cpu.IMM, cpu.PC);
    printf("IR: %02X RO0: %X RO1: %X\n", cpu.IR, cpu.RO0, cpu.RO1);
    printf("E: %X L: %X G: %X\n", cpu.E, cpu.L, cpu.G);
    
    printf("\nMemoria:\n   ");
    for (int i = 0; i < 16; i++) printf("%02X ", i);
    printf("\n");
    for (int i = 0; i < TAMANHO_MEMORIA; i += 16) {
        printf("%02X ", i);
        for (int j = 0; j < 16; j++) {
            printf("%02X ", memoria[i + j]);
        }
        printf("\n");
    }
}

// ----------------- CICLO DE MÁQUINA -----------------

void busca() {
    cpu.MBR = lerByte(cpu.PC);
}

void decodifica() {
    unsigned char byte0 = cpu.MBR & MASK_BYTE;
    
    cpu.IR = (byte0 >> 3) & MASK_OPCODE;
    cpu.RO0 = byte0 & MASK_REG;
    cpu.RO1 = 0;
    cpu.IMM = 0;

    if (cpu.IR == OP_HLT || cpu.IR == OP_NOP || cpu.IR == OP_NOT) {
        tamanhoInstrucaoAtual = 1;
    } 
    else if (cpu.IR >= OP_LDR && cpu.IR <= OP_XOR) {
        tamanhoInstrucaoAtual = 2;
        unsigned char byte1 = lerByte(cpu.PC + 1);
        cpu.RO1 = (byte1 >> 5) & MASK_REG;
        cpu.MBR = (byte0 << 8) | byte1; 
    } 
    else {
        tamanhoInstrucaoAtual = 3;
        unsigned char byte1 = lerByte(cpu.PC + 1);
        unsigned char byte2 = lerByte(cpu.PC + 2);
        cpu.IMM = (byte1 << 8) | byte2;
        cpu.MAR = cpu.IMM; 
        cpu.MBR = (byte0 << 16) | (byte1 << 8) | byte2;
    }
}

void executa() {
    unsigned short int endereco;

    switch (cpu.IR) {
        case OP_HLT: 
            halt = 1; 
            break;
        case OP_NOP: 
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_LDR: 
            endereco = cpu.reg[cpu.RO1];
            cpu.reg[cpu.RO0] = lerPalavra(endereco);
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_STR: 
            endereco = cpu.reg[cpu.RO1];
            escreverPalavra(endereco, cpu.reg[cpu.RO0]);
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_ADD: 
            cpu.reg[cpu.RO0] += cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_SUB: 
            cpu.reg[cpu.RO0] -= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_MUL: 
            cpu.reg[cpu.RO0] *= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_DIV: 
            if(cpu.reg[cpu.RO1] != 0) 
                cpu.reg[cpu.RO0] /= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_CMP: 
            cpu.E = (cpu.reg[cpu.RO0] == cpu.reg[cpu.RO1]) ? 1 : 0;
            cpu.L = (cpu.reg[cpu.RO0] < cpu.reg[cpu.RO1]) ? 1 : 0;
            cpu.G = (cpu.reg[cpu.RO0] > cpu.reg[cpu.RO1]) ? 1 : 0;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_MOVR: 
            cpu.reg[cpu.RO0] = cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_AND: 
            cpu.reg[cpu.RO0] &= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_OR: 
            cpu.reg[cpu.RO0] |= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_XOR: 
            cpu.reg[cpu.RO0] ^= cpu.reg[cpu.RO1];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_NOT: 
            cpu.reg[cpu.RO0] = !cpu.reg[cpu.RO0];
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JE: 
            if (cpu.E == 1) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JNE: 
            if (cpu.E == 0) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JL: 
            if (cpu.L == 1) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JLE: 
            if (cpu.E == 1 || cpu.L == 1) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JG: 
            if (cpu.G == 1) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JGE: 
            if (cpu.E == 1 || cpu.G == 1) cpu.PC = cpu.IMM;
            else cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_JMP: 
            cpu.PC = cpu.IMM; 
            break;
        case OP_LD: 
            cpu.reg[cpu.RO0] = lerPalavra(cpu.IMM);
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_ST: 
            escreverPalavra(cpu.IMM, cpu.reg[cpu.RO0]);
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_MOVI: 
            cpu.reg[cpu.RO0] = cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_ADDI: 
            cpu.reg[cpu.RO0] += cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_SUBI: 
            cpu.reg[cpu.RO0] -= cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_MULI: 
            cpu.reg[cpu.RO0] *= cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_DIVI: 
            if(cpu.IMM != 0)
                cpu.reg[cpu.RO0] /= cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_LSH: 
            cpu.reg[cpu.RO0] = cpu.reg[cpu.RO0] << cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
        case OP_RSH: 
            cpu.reg[cpu.RO0] = cpu.reg[cpu.RO0] >> cpu.IMM;
            cpu.PC += tamanhoInstrucaoAtual; 
            break;
    }
}

int main() {
    memset(memoria, 0xFF, sizeof(memoria));
    memset(&cpu, 0, sizeof(Registrador));

    carregarArquivo("instrucoes.txt");

    getchar();

    while (!halt) {
        busca();
        decodifica();
        executa();

        imprimeEstado();

        if (!halt) {
            printf("\nPressione ENTER para o proximo ciclo ou CTRL+C para sair.\n");
            getchar();
        }
    }

    printf("\nPrograma finalizado (HLT executado).\n");
    return 0;
}