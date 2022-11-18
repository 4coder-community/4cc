/*
 * Instruction-level simulator for the LC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5
#define HALT 6
#define NOOP 7

typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

void printState(stateType *);
void run(stateType);
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

    /* read machine-code file into instruction/data memory (starting at
	address 0) */

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
	printf("error: can't open file %s\n", argv[1]);
	perror("fopen");
	exit(1);
    }

    for (state.numMemory=0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
	state.numMemory++) {
	if (state.numMemory >= NUMMEMORY) {
	    printf("exceeded memory size\n");
	    exit(1);
	}
	if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
	    printf("error in reading address %d\n", state.numMemory);
	    exit(1);
	}
	printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    printf("\n");
    
    /* run never returns */
    run(state);

    return(0);
}

void
run(stateType state)
{
    int arg0, arg1, arg2, addressField;
    int instructions=0;
    int opcode;
    int maxMem=-1;	/* highest memory address touched during run */

    for (; 1; instructions++) { /* infinite loop, exits when it executes halt */
	printState(&state);

	if (state.pc < 0 || state.pc >= NUMMEMORY) {
	    printf("pc went out of the memory range\n");
	    exit(1);
	}

	maxMem = (state.pc > maxMem)?state.pc:maxMem;
