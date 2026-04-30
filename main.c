# include <stdio.h>
# include <stdlib.h>
#include <string.h>
#include "defines.h"


Registrador registrador;

unsigned char MEM[256]; 

void inicializaReg(){
    registrador.PC=0;
    registrador.MAR=0;
    registrador.IR=0;
    registrador.IMM=0;
    registrador.IBR=0;
    registrador.E=0;
    registrador.L=0;
    registrador.G=0;
    registrador.LR=0;
    registrador.A=0;
    registrador.B=0;
    registrador.T=0;
    registrador.R01=0;
    registrador.R02=0;

        
}

void inicializaMemoria()
{
    for (int i = 0; i < tamanhoMemoria; i++)
    {
        MEM[i] = hlt;
    }
}


void imprimeMemoria()
{
    const int colunasPorLinha = 6;

    printf("\nMEMORIA:\n");
    for (int endereco = 0; endereco < tamanhoMemoria; endereco++)
    {
        printf("%04X: 0x%04X\t", endereco, MEM[endereco]);
        if ((endereco + 1) % colunasPorLinha == 0)
        {
            printf("\n");
        }
    }
}


void busca(){
    printf("Buscando instrução na memória...\n");
}

void executa(){

    printf("Executando instrução...\n");
}

void decodifica(){
    printf("Decodificando instrução...\n");
}







int main (){


    inicializaReg();
    inicializaMemoria();
    imprimeMemoria();
}