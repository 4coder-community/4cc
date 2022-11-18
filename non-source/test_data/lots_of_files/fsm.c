/* FSM for LC */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000
 
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int memoryAddress;
    int memoryData;
    int instrReg;
    int aluOperand;
    int aluResult;
    int numMemory;
} stateType;
 
void printState(stateType *, char *);
void run(stateType);
int memoryAccess(stateType *, int);
int convertNum(int);
 
int
main(int argc, char *argv[])
{
    int i;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
 
    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }
 
    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
        state.mem[i] = 0;
    }
    for (i=0; i<NUMREGS; i++) {
        state.reg[i] = 0;
    }
 
    state.pc=0;
 
    /* 
     * read machine-code file into instruction/data memory 
     * (starting at address 0)
     */
 
    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s\n", argv[1]);
        perror("fopen");
        exit(1);
    }
 
    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
        state.numMemory++) {
        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
 
    printf("\n");
 
    /* run never returns */
    run(state);
 
    return 0;
}
 
void
printState_(stateType *statePtr, char *stateName, int showAll)
{
    int i;
    static int cycle = 0;
    printf("\n@@@\nstate %s (cycle %d)\n", stateName, cycle++);
    if (showAll){
        printf("\tpc %d\n", statePtr->pc);
        printf("\tmemory:\n");
        for (i=0; i<statePtr->numMemory; i++) {
            printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
        }
        printf("\tregisters:\n");
        for (i=0; i<NUMREGS; i++) {
            printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
        }
        printf("\tinternal registers:\n");
        printf("\t\tmemoryAddress %d\n", statePtr->memoryAddress);
        printf("\t\tmemoryData %d\n", statePtr->memoryData);
        printf("\t\tinstrReg %d\n", statePtr->instrReg);
        printf("\t\taluOperand %d\n", statePtr->aluOperand);
        printf("\t\taluResult %d\n", statePtr->aluResult);
    }
}

#define printState(s,n) printState_(s,n,1)

void
printStateReal(stateType *statePtr, char *stateName)
{
    printState_(statePtr, stateName, 1);
}

/*
 * Access memory:
 * readFlag=1 ==> read from memory
 * readFlag=0 ==> write to memory
 * Return 1 if the memory operation was successful, otherwise return 0
 */
int
memoryAccess(stateType *statePtr, int readFlag)
{
    static int lastAddress = -1;
    static int lastReadFlag = 0;
    static int lastData = 0;
    static int delay = 0;
 
    if (statePtr->memoryAddress < 0 || statePtr->memoryAddress >= NUMMEMORY) {
        printf("memory address out of range\n");
        exit(1);
    }
 
 /*
  * If this is a new access, reset the delay clock.
  */
    if ( (statePtr->memoryAddress != lastAddress) ||
             (readFlag != lastReadFlag) ||
             (readFlag == 0 && lastData != statePtr->memoryData) ) {
        delay = statePtr->memoryAddress % 3;
        lastAddress = statePtr->memoryAddress;
        lastReadFlag = readFlag;
        lastData = statePtr->memoryData;
    }
 
    if (delay == 0) {
        /* memory is ready */
        if (readFlag) {
            statePtr->memoryData = statePtr->mem[statePtr->memoryAddress];
        } else {
            statePtr->mem[statePtr->memoryAddress] = statePtr->memoryData;
        }
        return 1;
    } else {
        /* memory is not ready */
        delay--;
        return 0;
    }
}
 
int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit integer */
    if (num & (1 << 15) ) {
        num -= (1 << 16);
    }
    return num;
}

#define OP_ADD 0
#define OP_NAND 1
#define OP_LW 2
#define OP_SW 3
#define OP_BEQ 4
#define OP_JALR 5
#define OP_HALT 6
#define OP_NOOP 7

void 
run(stateType state)
{
    int bus;

fetch:
    printState(&state, "fetch");
    bus = state.pc;
    state.memoryAddress = bus;
    state.pc++;
    goto fetch2;

fetch2:
    printState(&state, "fetch2");
    if (memoryAccess(&state, 1)) goto decode1;
    goto fetch2;

decode1:
    printState(&state, "decode1");
    bus = state.memoryData;
    state.instrReg = bus;
    goto decode2;

decode2:
    printState(&state, "decode2");
    bus = state.reg[(state.instrReg >> 19) & 7];
    state.aluOperand = bus;
    if ((state.instrReg>>22) == OP_ADD) goto add1;
    if ((state.instrReg>>22) == OP_NAND) goto nand1;
    if ((state.instrReg>>22) == OP_LW) goto lw1;
    if ((state.instrReg>>22) == OP_SW) goto sw1;
    if ((state.instrReg>>22) == OP_BEQ) goto beq1;
    if ((state.instrReg>>22) == OP_JALR) goto jalr1;
    if ((state.instrReg>>22) == OP_HALT) goto halt;
    if ((state.instrReg>>22) == OP_NOOP) goto fetch;
    exit(1);

add1:
    printState(&state, "add1");
    bus = state.reg[(state.instrReg >> 16) & 7];
    state.aluResult = state.aluOperand + bus;
    goto alu_finish;

nand1:
    printState(&state, "nand1");
    bus = state.reg[(state.instrReg >> 16) & 7];
    state.aluResult = ~(state.aluOperand & bus);
    goto alu_finish;

alu_finish:
    printState(&state, "alu_finish");
    bus = state.aluResult;
    state.reg[(state.instrReg) & 7] = bus;
    goto fetch;

lw1:
    printState(&state, "lw1");
    bus = convertNum(state.instrReg & 0xFFFF);
    state.aluResult = state.aluOperand + bus;
    goto lw2;

lw2:
    printState(&state, "lw2");
    bus = state.aluResult;
    state.memoryAddress = bus;
    goto lw3;

lw3:
    printState(&state, "lw3");
    if (memoryAccess(&state, 1)) goto lw4;
    goto lw3;

lw4:
    printState(&state, "lw4");
    bus = state.memoryData;
    state.reg[(state.instrReg >> 16) & 7] = bus;
    goto fetch;

sw1:
    printState(&state, "sw1");
    bus = convertNum(state.instrReg & 0xFFFF);
    state.aluResult = state.aluOperand + bus;
    goto sw2;

sw2:
    printState(&state, "sw2");
    bus = state.aluResult;
    state.memoryAddress = bus;
    goto sw3;

sw3:
    printState(&state, "sw3");
    bus = state.reg[(state.instrReg >> 16) & 7];
    state.memoryData = bus;
    goto sw4;

sw4:
    printState(&state, "sw4");
    if (memoryAccess(&state, 0)) goto fetch;
    goto sw4;

beq1:
    printState(&state, "beq1");
    bus = state.reg[(state.instrReg >> 16) & 7];
    state.aluResult = state.aluOperand - bus;
    goto beq2;

beq2:
    printState(&state, "beq2");
    bus = convertNum(state.instrReg & 0xFFFF);
    state.aluOperand = bus;
    if (state.aluResult == 0) goto beq3;
    goto fetch;

beq3:
    printState(&state, "beq3");
    bus = state.pc;
    state.aluResult = state.aluOperand + bus;
    goto beq4;

beq4:
    printState(&state, "beq4");
    bus = state.aluResult;
    state.pc = bus;
    goto fetch;

jalr1:
    printState(&state, "jalr1");
    bus = state.pc;
    state.reg[(state.instrReg >> 16) & 7] = bus;
    goto jalr2;

jalr2:
    printState(&state, "jalr2");
    bus = state.reg[(state.instrReg >> 19) & 7];
    state.pc = bus;
    goto fetch;

halt:
    printStateReal(&state, "halt");
    exit(0);
}

