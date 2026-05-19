#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"

#define MAX_LINHA 160
#define MAX_CICLOS_PADRAO 10000

typedef struct
{
    unsigned char bytes[3];
    unsigned char tamanho;
} InstrucaoCodificada;

static Registrador cpu;
static unsigned char memoria[TAMANHO_MEMORIA];
static unsigned char haltExecutado = 0;
static unsigned char tamanhoInstrucaoAtual = 0;

static const char *nomeOpcode[] = {
    "hlt",  "nop",  "ldr",  "str",  "add",  "sub", "mul", "div",
    "cmp",  "movr", "and",  "or",   "xor",  "not", "je",  "jne",
    "jl",   "jle",  "jg",   "jge",  "jmp",  "ld",  "st",  "movi",
    "addi", "subi", "muli", "divi", "lsh",  "rsh"};

static void erro(const char *mensagem)
{
    fprintf(stderr, "Erro: %s\n", mensagem);
    exit(EXIT_FAILURE);
}

static void erroLinha(int linha, const char *mensagem)
{
    fprintf(stderr, "Erro na linha %d: %s\n", linha, mensagem);
    exit(EXIT_FAILURE);
}

static char *trim(char *texto)
{
    char *fim;

    while (isspace((unsigned char)*texto))
        texto++;

    if (*texto == '\0')
        return texto;

    fim = texto + strlen(texto) - 1;
    while (fim > texto && isspace((unsigned char)*fim))
        fim--;

    fim[1] = '\0';
    return texto;
}

static void paraMinusculo(char *texto)
{
    while (*texto)
    {
        *texto = (char)tolower((unsigned char)*texto);
        texto++;
    }
}

static unsigned int parseHex(const char *texto, unsigned int maximo, int linha, const char *campo)
{
    char *fim;
    unsigned long valor;

    errno = 0;
    valor = strtoul(texto, &fim, 16);

    while (isspace((unsigned char)*fim))
        fim++;

    if (errno != 0 || *texto == '\0' || *fim != '\0' || valor > maximo)
    {
        char mensagem[120];
        snprintf(mensagem, sizeof(mensagem), "valor hexadecimal invalido em %s: %s", campo, texto);
        erroLinha(linha, mensagem);
    }

    return (unsigned int)valor;
}

static int parseRegistrador(const char *texto, int linha)
{
    if (texto == NULL || strlen(texto) != 2 || texto[0] != 'r' || texto[1] < '0' || texto[1] > '7')
        erroLinha(linha, "registrador invalido; use r0 a r7");

    return texto[1] - '0';
}

static int opcodePorNome(const char *mnemonico)
{
    int i;

    for (i = 0; i < (int)(sizeof(nomeOpcode) / sizeof(nomeOpcode[0])); i++)
    {
        if (strcmp(mnemonico, nomeOpcode[i]) == 0)
            return i;
    }

    return -1;
}

static int instrucaoRegReg(int opcode)
{
    return opcode >= OP_LDR && opcode <= OP_XOR;
}

static int instrucaoDesvio(int opcode)
{
    return opcode >= OP_JE && opcode <= OP_JMP;
}

static int instrucaoRegImediatoOuEndereco(int opcode)
{
    return opcode >= OP_LD && opcode <= OP_RSH;
}

static unsigned char tamanhoInstrucao(int opcode)
{
    if (opcode == OP_HLT || opcode == OP_NOP || opcode == OP_NOT)
        return 1;

    if (instrucaoRegReg(opcode))
        return 2;

    if (instrucaoDesvio(opcode) || instrucaoRegImediatoOuEndereco(opcode))
        return 3;

    return 0;
}

static void validaEnderecoByte(unsigned int endereco)
{
    if (endereco >= TAMANHO_MEMORIA)
        erro("endereco de memoria fora do intervalo 0x00 a 0xFF");
}

static void validaEnderecoPalavra(unsigned int endereco)
{
    if (endereco + 1 >= TAMANHO_MEMORIA)
        erro("palavra de 16 bits ultrapassa o fim da memoria");
}

static unsigned char lerByte(unsigned short int endereco)
{
    validaEnderecoByte(endereco);
    cpu.MAR = endereco;
    cpu.MBR = memoria[endereco];
    return (unsigned char)(cpu.MBR & MASK_BYTE);
}

static void escreverByte(unsigned short int endereco, unsigned char valor)
{
    validaEnderecoByte(endereco);
    cpu.MAR = endereco;
    cpu.MBR = valor;
    memoria[endereco] = (unsigned char)(cpu.MBR & MASK_BYTE);
}

static unsigned short int lerPalavra(unsigned short int endereco)
{
    unsigned char alto;
    unsigned char baixo;

    validaEnderecoPalavra(endereco);
    alto = lerByte(endereco);
    baixo = lerByte((unsigned short int)(endereco + 1));
    cpu.MAR = endereco;
    cpu.MBR = ((unsigned int)alto << 8) | baixo;

    return (unsigned short int)(cpu.MBR & MASK_WORD);
}

static void escreverPalavra(unsigned short int endereco, unsigned short int valor)
{
    validaEnderecoPalavra(endereco);
    cpu.MAR = endereco;
    cpu.MBR = valor;
    escreverByte(endereco, (unsigned char)((valor >> 8) & MASK_BYTE));
    escreverByte((unsigned short int)(endereco + 1), (unsigned char)(valor & MASK_BYTE));
    cpu.MAR = endereco;
    cpu.MBR = valor;
}

static void gravarByteCarga(unsigned int endereco, unsigned char valor)
{
    validaEnderecoByte(endereco);
    memoria[endereco] = valor;
}

static void gravarPalavraCarga(unsigned int endereco, unsigned int valor)
{
    validaEnderecoPalavra(endereco);
    memoria[endereco] = (unsigned char)((valor >> 8) & MASK_BYTE);
    memoria[endereco + 1] = (unsigned char)(valor & MASK_BYTE);
}

static void inicializaRegistrador(void)
{
    int i;

    cpu.PC = 0;
    cpu.MBR = 0;
    cpu.MAR = 0;
    cpu.IR = 0;
    cpu.RO0 = 0;
    cpu.RO1 = 0;
    cpu.IMM = 0;
    cpu.E = 0;
    cpu.L = 0;
    cpu.G = 0;

    for (i = 0; i < QTD_REGISTRADORES; i++)
        cpu.reg[i] = 0;

    haltExecutado = 0;
    tamanhoInstrucaoAtual = 0;
}

static void inicializaMemoria(void)
{
    int i;

    for (i = 0; i < TAMANHO_MEMORIA; i++)
        memoria[i] = 0xFF;
}

static void imprimeEstadoCPU(void)
{
    printf("\nCPU:\n");
    printf("R0: %04X R1: %04X R2: %04X R3: %04X\n",
           cpu.reg[0], cpu.reg[1], cpu.reg[2], cpu.reg[3]);
    printf("R4: %04X R5: %04X R6: %04X R7: %04X\n",
           cpu.reg[4], cpu.reg[5], cpu.reg[6], cpu.reg[7]);
    printf("MBR: %08X MAR: %04X IMM: %04X PC: %04X\n",
           cpu.MBR, cpu.MAR, cpu.IMM, cpu.PC);
    printf("IR: %02X RO0: %X RO1: %X\n", cpu.IR, cpu.RO0, cpu.RO1);
    printf("E: %X L: %X G: %X\n", cpu.E, cpu.L, cpu.G);
}

static void imprimeMemoria(void)
{
    int linha;
    int coluna;

    printf("\nMemoria:\n");
    printf("   ");
    for (coluna = 0; coluna < 16; coluna++)
        printf("%02X ", coluna);
    printf("\n");

    for (linha = 0; linha < TAMANHO_MEMORIA; linha += 16)
    {
        printf("%02X ", linha);
        for (coluna = 0; coluna < 16; coluna++)
            printf("%02X ", memoria[linha + coluna]);
        printf("\n");
    }
}

static void aguardarEnter(void)
{
    int c;

    printf("\nPressione Enter para iniciar o proximo ciclo de maquina ou CTRL+C para finalizar.\n");
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

static char *proximoOperando(char **resto)
{
    char *operando;

    operando = strtok(*resto, ",");
    if (operando == NULL)
        return NULL;

    *resto = NULL;
    return trim(operando);
}

static InstrucaoCodificada montarInstrucao(char *conteudo, int linha)
{
    char *mnemonico;
    char *operandos;
    char *op0;
    char *op1;
    int opcode;
    unsigned int valor;
    InstrucaoCodificada inst;

    paraMinusculo(conteudo);
    mnemonico = strtok(conteudo, " \t\r\n");
    operandos = strtok(NULL, "\r\n");

    if (mnemonico == NULL)
        erroLinha(linha, "instrucao vazia");

    opcode = opcodePorNome(mnemonico);
    if (opcode < 0)
        erroLinha(linha, "mnemonico desconhecido");

    inst.bytes[0] = 0;
    inst.bytes[1] = 0;
    inst.bytes[2] = 0;
    inst.tamanho = tamanhoInstrucao(opcode);

    if (inst.tamanho == 0)
        erroLinha(linha, "opcode invalido");

    if (opcode == OP_HLT || opcode == OP_NOP)
    {
        if (operandos != NULL && *trim(operandos) != '\0')
            erroLinha(linha, "hlt e nop nao recebem operandos");
        inst.bytes[0] = (unsigned char)(opcode << 3);
        return inst;
    }

    if (operandos == NULL)
        erroLinha(linha, "faltam operandos");

    operandos = trim(operandos);

    if (opcode == OP_NOT)
    {
        op0 = trim(operandos);
        if (strchr(op0, ',') != NULL)
            erroLinha(linha, "not recebe apenas um registrador");
        inst.bytes[0] = (unsigned char)((opcode << 3) | parseRegistrador(op0, linha));
        return inst;
    }

    op0 = proximoOperando(&operandos);
    op1 = strtok(NULL, ",");

    if (op0 == NULL || op1 == NULL)
        erroLinha(linha, "instrucao precisa de dois operandos");

    op1 = trim(op1);
    if (strtok(NULL, ",") != NULL)
        erroLinha(linha, "operandos em excesso");

    if (instrucaoRegReg(opcode))
    {
        inst.bytes[0] = (unsigned char)((opcode << 3) | parseRegistrador(op0, linha));
        inst.bytes[1] = (unsigned char)(parseRegistrador(op1, linha) << 5);
        return inst;
    }

    if (instrucaoDesvio(opcode))
    {
        if (op1 != NULL)
            erroLinha(linha, "desvios recebem apenas um endereco");
    }

    if (instrucaoRegImediatoOuEndereco(opcode))
    {
        valor = parseHex(op1, MASK_WORD, linha, "imediato/endereco");
        inst.bytes[0] = (unsigned char)((opcode << 3) | parseRegistrador(op0, linha));
        inst.bytes[1] = (unsigned char)((valor >> 8) & MASK_BYTE);
        inst.bytes[2] = (unsigned char)(valor & MASK_BYTE);
        return inst;
    }

    erroLinha(linha, "formato de instrucao nao implementado");
    return inst;
}

static InstrucaoCodificada montarDesvio(char *conteudo, int linha)
{
    char *mnemonico;
    char *operando;
    int opcode;
    unsigned int valor;
    InstrucaoCodificada inst;

    paraMinusculo(conteudo);
    mnemonico = strtok(conteudo, " \t\r\n");
    operando = strtok(NULL, "\r\n");

    if (mnemonico == NULL)
        erroLinha(linha, "instrucao vazia");

    opcode = opcodePorNome(mnemonico);
    if (!instrucaoDesvio(opcode))
        erroLinha(linha, "desvio invalido");

    if (operando == NULL)
        erroLinha(linha, "desvio sem endereco");

    operando = trim(operando);
    if (strchr(operando, ',') != NULL)
        erroLinha(linha, "desvio recebe apenas um endereco");

    valor = parseHex(operando, MASK_WORD, linha, "endereco de desvio");

    inst.tamanho = 3;
    inst.bytes[0] = (unsigned char)(opcode << 3);
    inst.bytes[1] = (unsigned char)((valor >> 8) & MASK_BYTE);
    inst.bytes[2] = (unsigned char)(valor & MASK_BYTE);

    return inst;
}

static InstrucaoCodificada codificarInstrucao(char *conteudo, int linha)
{
    char copia[MAX_LINHA];
    char *mnemonico;
    int opcode;

    strncpy(copia, conteudo, sizeof(copia) - 1);
    copia[sizeof(copia) - 1] = '\0';
    paraMinusculo(copia);
    mnemonico = strtok(copia, " \t\r\n");

    if (mnemonico == NULL)
        erroLinha(linha, "instrucao vazia");

    opcode = opcodePorNome(mnemonico);
    if (opcode < 0)
        erroLinha(linha, "mnemonico desconhecido");

    if (instrucaoDesvio(opcode))
        return montarDesvio(conteudo, linha);

    return montarInstrucao(conteudo, linha);
}

static unsigned short int carregarArquivo(const char *nomeArquivo)
{
    FILE *arquivo;
    char linhaOriginal[MAX_LINHA];
    unsigned short int pcInicial = 0;
    int encontrouInstrucao = 0;
    int numeroLinha = 0;

    arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL)
        erro("nao foi possivel abrir o arquivo de entrada");

    while (fgets(linhaOriginal, sizeof(linhaOriginal), arquivo) != NULL)
    {
        char linha[MAX_LINHA];
        char *comentario;
        char *enderecoTexto;
        char *tipoTexto;
        char *conteudo;
        unsigned int endereco;
        numeroLinha++;

        strncpy(linha, linhaOriginal, sizeof(linha) - 1);
        linha[sizeof(linha) - 1] = '\0';

        comentario = strchr(linha, '#');
        if (comentario != NULL)
            *comentario = '\0';

        if (*trim(linha) == '\0')
            continue;

        enderecoTexto = strtok(linha, ";");
        tipoTexto = strtok(NULL, ";");
        conteudo = strtok(NULL, "\n");

        if (enderecoTexto == NULL || tipoTexto == NULL || conteudo == NULL)
            erroLinha(numeroLinha, "formato esperado: endereco;tipo;conteudo");

        enderecoTexto = trim(enderecoTexto);
        tipoTexto = trim(tipoTexto);
        conteudo = trim(conteudo);
        endereco = parseHex(enderecoTexto, TAMANHO_MEMORIA - 1, numeroLinha, "endereco");

        if (strlen(tipoTexto) != 1)
            erroLinha(numeroLinha, "tipo deve ser i ou d");

        if (tolower((unsigned char)tipoTexto[0]) == 'd')
        {
            unsigned int valor = parseHex(conteudo, MASK_WORD, numeroLinha, "dado");
            gravarPalavraCarga(endereco, valor);
        }
        else if (tolower((unsigned char)tipoTexto[0]) == 'i')
        {
            int i;
            InstrucaoCodificada inst = codificarInstrucao(conteudo, numeroLinha);

            if (endereco + inst.tamanho > TAMANHO_MEMORIA)
                erroLinha(numeroLinha, "instrucao ultrapassa o fim da memoria");

            if (!encontrouInstrucao)
            {
                pcInicial = (unsigned short int)endereco;
                encontrouInstrucao = 1;
            }

            for (i = 0; i < inst.tamanho; i++)
                gravarByteCarga(endereco + i, inst.bytes[i]);
        }
        else
        {
            erroLinha(numeroLinha, "tipo invalido; use i para instrucao ou d para dado");
        }
    }

    fclose(arquivo);

    if (!encontrouInstrucao)
        erro("arquivo sem instrucoes");

    return pcInicial;
}

static void busca(void)
{
    cpu.MBR = lerByte(cpu.PC);
}

static void decodifica(void)
{
    unsigned char byte0 = (unsigned char)(cpu.MBR & MASK_BYTE);
    unsigned char byte1 = 0;
    unsigned char byte2 = 0;

    cpu.IR = (unsigned char)((byte0 >> 3) & MASK_OPCODE);
    cpu.RO0 = (unsigned char)(byte0 & MASK_REG);
    cpu.RO1 = 0;
    cpu.IMM = 0;
    tamanhoInstrucaoAtual = tamanhoInstrucao(cpu.IR);

    if (tamanhoInstrucaoAtual == 0)
        erro("opcode invalido encontrado na memoria");

    if (tamanhoInstrucaoAtual >= 2)
    {
        byte1 = lerByte((unsigned short int)(cpu.PC + 1));
        cpu.RO1 = (unsigned char)((byte1 >> 5) & MASK_REG);
    }

    if (tamanhoInstrucaoAtual == 3)
    {
        byte2 = lerByte((unsigned short int)(cpu.PC + 2));
        cpu.IMM = (unsigned short int)(((unsigned int)byte1 << 8) | byte2);
        cpu.MAR = cpu.IMM;
    }

    cpu.MBR = ((unsigned int)byte0 << 16) | ((unsigned int)byte1 << 8) | byte2;
}

static void avancarPC(void)
{
    cpu.PC = (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
}

static void executa(void)
{
    unsigned short int endereco;

    switch (cpu.IR)
    {
    case OP_HLT:
        haltExecutado = 1;
        break;

    case OP_NOP:
        avancarPC();
        break;

    case OP_LDR:
        endereco = cpu.reg[cpu.RO1];
        cpu.reg[cpu.RO0] = lerPalavra(endereco);
        avancarPC();
        break;

    case OP_STR:
        endereco = cpu.reg[cpu.RO1];
        escreverPalavra(endereco, cpu.reg[cpu.RO0]);
        avancarPC();
        break;

    case OP_ADD:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] + cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_SUB:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] - cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_MUL:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] * cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_DIV:
        if (cpu.reg[cpu.RO1] == 0)
            erro("divisao por zero");
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] / cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_CMP:
        cpu.E = (unsigned char)(cpu.reg[cpu.RO0] == cpu.reg[cpu.RO1]);
        cpu.L = (unsigned char)(cpu.reg[cpu.RO0] < cpu.reg[cpu.RO1]);
        cpu.G = (unsigned char)(cpu.reg[cpu.RO0] > cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_MOVR:
        cpu.reg[cpu.RO0] = cpu.reg[cpu.RO1];
        avancarPC();
        break;

    case OP_AND:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] & cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_OR:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] | cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_XOR:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] ^ cpu.reg[cpu.RO1]);
        avancarPC();
        break;

    case OP_NOT:
        cpu.reg[cpu.RO0] = (unsigned short int)(!cpu.reg[cpu.RO0]);
        avancarPC();
        break;

    case OP_JE:
        cpu.PC = cpu.E ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JNE:
        cpu.PC = !cpu.E ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JL:
        cpu.PC = cpu.L ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JLE:
        cpu.PC = (cpu.L || cpu.E) ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JG:
        cpu.PC = cpu.G ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JGE:
        cpu.PC = (cpu.G || cpu.E) ? cpu.IMM : (unsigned short int)(cpu.PC + tamanhoInstrucaoAtual);
        break;

    case OP_JMP:
        cpu.PC = cpu.IMM;
        break;

    case OP_LD:
        cpu.reg[cpu.RO0] = lerPalavra(cpu.IMM);
        avancarPC();
        break;

    case OP_ST:
        escreverPalavra(cpu.IMM, cpu.reg[cpu.RO0]);
        avancarPC();
        break;

    case OP_MOVI:
        cpu.reg[cpu.RO0] = cpu.IMM;
        avancarPC();
        break;

    case OP_ADDI:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] + cpu.IMM);
        avancarPC();
        break;

    case OP_SUBI:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] - cpu.IMM);
        avancarPC();
        break;

    case OP_MULI:
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] * cpu.IMM);
        avancarPC();
        break;

    case OP_DIVI:
        if (cpu.IMM == 0)
            erro("divisao por zero");
        cpu.reg[cpu.RO0] = (unsigned short int)(cpu.reg[cpu.RO0] / cpu.IMM);
        avancarPC();
        break;

    case OP_LSH:
        cpu.reg[cpu.RO0] = cpu.IMM >= 16 ? 0 : (unsigned short int)(cpu.reg[cpu.RO0] << cpu.IMM);
        avancarPC();
        break;

    case OP_RSH:
        cpu.reg[cpu.RO0] = cpu.IMM >= 16 ? 0 : (unsigned short int)(cpu.reg[cpu.RO0] >> cpu.IMM);
        avancarPC();
        break;

    default:
        erro("opcode sem execucao implementada");
    }
}

static void executarPrograma(int modoBatch, int maxCiclos)
{
    int ciclos = 0;

    while (!haltExecutado)
    {
        if (ciclos >= maxCiclos)
            erro("limite maximo de ciclos atingido; possivel loop infinito");

        busca();
        decodifica();
        executa();
        ciclos++;

        if (!modoBatch)
        {
            printf("\nCiclo %d finalizado.\n", ciclos);
            imprimeEstadoCPU();
            imprimeMemoria();
            if (!haltExecutado)
                aguardarEnter();
        }
    }

    if (modoBatch)
    {
        printf("\nExecucao finalizada em %d ciclo(s).\n", ciclos);
        imprimeEstadoCPU();
        imprimeMemoria();
    }
}

static void imprimirUso(const char *programa)
{
    printf("Uso: %s arquivo.txt [--batch] [--max-cycles N]\n", programa);
    printf("  --batch        executa sem pausas e imprime apenas o estado final\n");
    printf("  --max-cycles N define limite de ciclos para detectar loops infinitos\n");
}

int main(int argc, char **argv)
{
    const char *arquivoEntrada = "instrucoes.txt";
    int modoBatch = 0;
    int maxCiclos = MAX_CICLOS_PADRAO;
    int i;

    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--batch") == 0)
        {
            modoBatch = 1;
        }
        else if (strcmp(argv[i], "--max-cycles") == 0)
        {
            if (i + 1 >= argc)
                erro("--max-cycles precisa de um numero");
            maxCiclos = atoi(argv[++i]);
            if (maxCiclos <= 0)
                erro("--max-cycles invalido");
        }
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            imprimirUso(argv[0]);
            return EXIT_SUCCESS;
        }
        else
        {
            arquivoEntrada = argv[i];
        }
    }

    inicializaRegistrador();
    inicializaMemoria();
    cpu.PC = carregarArquivo(arquivoEntrada);

    if (!modoBatch)
    {
        printf("Programa carregado. PC inicial: %04X\n", cpu.PC);
        imprimeEstadoCPU();
        imprimeMemoria();
        aguardarEnter();
    }

    executarPrograma(modoBatch, maxCiclos);
    return EXIT_SUCCESS;
}
