#ifndef DEFINES_H
#define DEFINES_H

#define TAMANHO_MEMORIA 0x100
#define QTD_REGISTRADORES 8

#define MASK_BYTE 0xFF
#define MASK_WORD 0xFFFF
#define MASK_OPCODE 0x1F
#define MASK_REG 0x07

typedef enum {
    OP_HLT = 0x00, 
    OP_NOP = 0x01, 
    OP_LDR = 0x02, 
    OP_STR = 0x03,
    OP_ADD = 0x04, 
    OP_SUB = 0x05, 
    OP_MUL = 0x06, 
    OP_DIV = 0x07,
    OP_CMP = 0x08, 
    OP_MOVR = 0x09, 
    OP_AND = 0x0A, 
    OP_OR = 0x0B,
    OP_XOR = 0x0C, 
    OP_NOT = 0x0D, 
    OP_JE = 0x0E, 
    OP_JNE = 0x0F,
    OP_JL = 0x10, 
    OP_JLE = 0x11, 
    OP_JG = 0x12, 
    OP_JGE = 0x13,
    OP_JMP = 0x14, 
    OP_LD = 0x15, 
    OP_ST = 0x16, 
    OP_MOVI = 0x17,
    OP_ADDI = 0x18, 
    OP_SUBI = 0x19, 
    OP_MULI = 0x1A, 
    OP_DIVI = 0x1B,
    OP_LSH = 0x1C, 
    OP_RSH = 0x1D
} Opcode;

typedef struct {
    unsigned short int PC;
    unsigned int MBR;
    unsigned short int MAR;
    unsigned char IR;
    unsigned char RO0;
    unsigned char RO1;
    unsigned short int IMM;
    unsigned char E;
    unsigned char L;
    unsigned char G;
    unsigned short int reg[QTD_REGISTRADORES]; // Usando o define aqui!
} Registrador;

#endif