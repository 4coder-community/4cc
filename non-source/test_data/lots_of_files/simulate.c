/* EECS 370 LC-2K Instruction-level simulator */

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
    int numMemory;
} stateType;

void printState(stateType *);

#define OP_FILL -1
#define OP_ADD 0
#define OP_NAND 1
#define OP_LW 2
#define OP_SW 3
#define OP_BEQ 4
#define OP_JALR 5
#define OP_HALT 6
#define OP_NOOP 7

#define SHIFT_OP 22
#define SHIFT_A 19
#define SHIFT_B 16
#define SHIFT_C 0

#define MASK_OP (7 << SHIFT_OP)
#define MASK_A (7 << SHIFT_A)
#define MASK_B (7 << SHIFT_B)
#define MASK_C (0xFFFF)

void
decode_insanity(int ins, int *o, int *a, int *b, int *c){
    *o = (ins & (MASK_OP)) >> SHIFT_OP;
    *a = (ins & (MASK_A)) >> SHIFT_A;
    *b = (ins & (MASK_B)) >> SHIFT_B;
    *c = (ins & (MASK_C)) >> SHIFT_C;
    if (*c & 0x8000) *c |= 0xFFFF0000;
}

int DID_NOOP;
int
execute_insanity(stateType *state, int *e){
    int r;
    int o,a,b,c;
    int m,pc;
    
    pc = state->pc++;
    decode_insanity(state->mem[pc], &o, &a, &b, &c);
    
    DID_NOOP = 0;
    r = 1;
    switch (o){
    case OP_ADD:
        state->reg[c] = state->reg[a] + state->reg[b];
        break;
        
    case OP_NAND:
        state->reg[c] = ~(state->reg[a] & state->reg[b]);
        break;
        
    case OP_LW:
        m = state->reg[a] + c;
        state->reg[b] = state->mem[m];
        break;
        
    case OP_SW:
        m = state->reg[a] + c;
        state->mem[m] = state->reg[b];
        break;
        
    case OP_BEQ:
        if (state->reg[a] == state->reg[b]) state->pc += c;
        break;
        
    case OP_JALR:
        state->reg[b] = pc+1;
        state->pc = state->reg[a];
        break;
        
    case OP_HALT:
        r = 0;
        break;
        
    case OP_NOOP: DID_NOOP = 1; break;
        
    default: r = 0; *e = 1; break;
    }
    
    return (r);
}

int
main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;
    int cont, e, step_count;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    state.pc = 0;
    memset(state.reg, 0, sizeof(int)*NUMREGS);
    memset(state.mem, 0, sizeof(int)*NUMMEMORY);
    
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    
	step_count = 0;
    cont = 1;
    while (cont){
		printf("steps: %d\n", step_count++);
        cont = execute_insanity(&state, &e);
        printState(&state);
        int x = 0;
    }
    // if (e) { /*there was an error*/ }
    printState(&state);
	printf("steps: %d\n", step_count++);
    
    return(0);
}

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
}

