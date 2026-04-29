#define hlt   0b00000
#define nop   0b00001

#define ldr   0b00010
#define str   0b00011
#define add   0b00100
#define sub   0b00101
#define mul   0b00110
#define div   0b00111
#define cmp   0b01000
#define movr  0b01001
#define and   0b01010
#define or    0b01011
#define xor   0b01100
#define not   0b01101

#define je    0b01110
#define jne   0b01111
#define jl    0b10000
#define jle   0b10001
#define jg    0b10010
#define jge   0b10011
#define jmp   0b10100

#define ld    0b10101
#define st    0b10110

#define movi  0b10111
#define addi  0b11000
#define subi  0b11001
#define muli  0b11010
#define divi  0b11011
#define lsh   0b11100
#define rsh   0b11101

typedef struct
{
    unsigned short int PC;   // Program Counter
    unsigned int MBR;        // Memory Buffer Register
    unsigned short int MAR;  // Memory Address Register
    unsigned char IR;        // Instruction Register
    unsigned short int IMM;  // Immediate
    unsigned short int IBR;  // Instruction Buffer Register

    unsigned short int RO0; // Register operand 0
    unsigned short int R01; // Register operand 1

    unsigned char E, L, G;   // Flags: Equal, Lower, Greater
    unsigned char LR;        // Left/Right control register

    unsigned short int A;    // General-purpose register A
    unsigned short int B;    // General-purpose register B
    unsigned short int T;    // Temporary register

} Registrador;