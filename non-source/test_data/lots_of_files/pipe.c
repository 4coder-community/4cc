/* PIPE for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR will not implemented for Project 3 */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDStruct {
	int instr;
	int pcPlus1;
} IFIDType;

typedef struct IDEXStruct {
	int instr;
	int pcPlus1;
	int readRegA;
	int readRegB;
	int offset;
} IDEXType;

typedef struct EXMEMStruct {
	int instr;
	int branchTarget;
	int aluResult;
	int readRegB;
} EXMEMType;

typedef struct MEMWBStruct {
	int instr;
	int writeData;
} MEMWBType;

typedef struct WBENDStruct {
	int instr;
	int writeData;
} WBENDType;

typedef struct stateStruct {
	int pc;
	int instrMem[NUMMEMORY];
	int dataMem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles; /* number of cycles run so far */
} stateType;

void run();
void printState(stateType*);
void printInstruction(int);
int convertNum(int);

stateType state;
stateType newState;

int
main(int argc, char *argv[])
{
    char line[MAXLINELENGTH];
    FILE *filePtr;
    int cont, e, step_count;

    if (argc != 2) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s\n", argv[1]);
        perror("fopen");
        exit(1);
    }

    state.pc = 0;
    state.cycles = 0;
    memset(state.reg, 0, sizeof(int)*NUMREGS);
    memset(state.dataMem, 0, sizeof(int)*NUMMEMORY);
    memset(state.instrMem, 0, sizeof(int)*NUMMEMORY);
    state.IFID.instr = NOOPINSTRUCTION;
    state.IDEX.instr = NOOPINSTRUCTION;
    state.EXMEM.instr = NOOPINSTRUCTION;
    state.MEMWB.instr = NOOPINSTRUCTION;
    state.WBEND.instr = NOOPINSTRUCTION;
    
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
            state.numMemory++) {

        if (sscanf(line, "%d", state.dataMem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        state.instrMem[state.numMemory] = state.dataMem[state.numMemory];
        printf("memory[%d]=%d\n", state.numMemory, state.dataMem[state.numMemory]);
    }

    newState = state;
    run();
    
    return(0);
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

void
printState(stateType *statePtr)
{
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles);
    printf("\tpc %d\n", statePtr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IFID.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->IDEX.instr);
	printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1);
	printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA);
	printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB);
	printf("\t\toffset %d\n", statePtr->IDEX.offset);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->EXMEM.instr);
	printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget);
	printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult);
	printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->MEMWB.instr);
	printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(statePtr->WBEND.instr);
	printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int
field0(int instruction)
{
	return( (instruction>>19) & 0x7);
}

int
field1(int instruction)
{
	return( (instruction>>16) & 0x7);
}

int
field2(int instruction)
{
	return(instruction & 0xFFFF);
}

int
opcode(int instruction)
{
	return(instruction>>22);
}

void
printInstruction(int instr)
{
	char opcodeString[10];
	
	if (opcode(instr) == ADD) {
		strcpy(opcodeString, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodeString, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodeString, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodeString, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodeString, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodeString, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodeString, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodeString, "noop");
	} else {
		strcpy(opcodeString, "data");
    }
    printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
		field2(instr));
}

int memstage_mux(int reg, int hazard_free_value){
    int result;
    result = hazard_free_value;
    
    assert(((state.EXMEM.instr >> 22) & 7) == SW);
    if (((state.MEMWB.instr >> 22) & 7) == LW){
        if (((state.MEMWB.instr >> 16) & 7) == reg){
            result = state.MEMWB.writeData;
        }
    }

    return result;
}

int exstage_mux(int reg, int hazard_free_value){
    int resolved;
    int result;
    resolved = 0;
    result = hazard_free_value;
    
    switch ((state.EXMEM.instr >> 22) & 7){
    case ADD: case NAND:
        if ((state.EXMEM.instr & 7) == reg){
            resolved = 1;
            result = state.EXMEM.aluResult;
        }
        break;
    case LW:
        if (((state.EXMEM.instr >> 16) & 7) == reg){
            assert((state.IDEX.instr >> 22) & 7 == SW);
        }
        break;
    }
    
    if (!resolved){
        switch ((state.MEMWB.instr >> 22) & 7){
        case ADD: case NAND:
            if (((state.MEMWB.instr) & 7) == reg){
                resolved = 1;
                result = state.MEMWB.writeData;
            }
            break;
        case LW:
            if (((state.MEMWB.instr >> 16) & 7) == reg){
                resolved = 1;
                result = state.MEMWB.writeData;
            }
            break;
        }

        if (!resolved){
            switch ((state.WBEND.instr >> 22) & 7){
            case ADD: case NAND:
                if (((state.WBEND.instr) & 7) == reg){
                    resolved = 1;
                    result = state.WBEND.writeData;
                }
                break;
            case LW:
                if (((state.WBEND.instr >> 16) & 7) == reg){
                    resolved = 1;
                    result = state.WBEND.writeData;
                }
                break;
            }
        }
    }
    
    return result;
}

void run(){
    int EXMuxTop, EXMuxBottom;
    int stalled;
    int lw_write_reg;
    
    while (1){
        printState(&state);

        /* check for halt */
        if (opcode(state.MEMWB.instr) == HALT) {
            printf("machine halted\n");
            printf("total of %d cycles executed\n", state.cycles);
            exit(0);
        }

        newState = state;
        newState.cycles++;
        
        stalled = 0;

        if (((state.IDEX.instr >> 22) & 7) == LW){
            lw_write_reg = (state.IDEX.instr >> 16) & 7;
            switch ((state.IFID.instr >> 22) & 7){
            case ADD: case NAND: case BEQ:
                if (((state.IFID.instr >> 19) & 7) == lw_write_reg) stalled = 1;
                if (((state.IFID.instr >> 16) & 7) == lw_write_reg) stalled = 1;
                break;
                
            case LW: case SW:
                if (((state.IFID.instr >> 19) & 7) == lw_write_reg) stalled = 1;
                break;
            }
        }
        
        /* --------------------- IF stage --------------------- */
        if (!stalled){
            newState.IFID.instr = state.instrMem[state.pc];
            newState.IFID.pcPlus1 = newState.pc + 1;
            newState.pc += 1;
        }
        else{
            newState.IFID.instr = state.IFID.instr;
            newState.IFID.pcPlus1 = state.IFID.pcPlus1;
            newState.pc = state.pc;
        }

        /* --------------------- ID stage --------------------- */
        if (!stalled){
            newState.IDEX.instr = state.IFID.instr;
        }
        else{
            newState.IDEX.instr = NOOPINSTRUCTION;
        }
        newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
        newState.IDEX.readRegA = state.reg[(state.IFID.instr >> 19) & 7];
        newState.IDEX.readRegB = state.reg[(state.IFID.instr >> 16) & 7];
        newState.IDEX.offset = convertNum(state.IFID.instr & 0xFFFF);
        
        /* --------------------- EX stage --------------------- */
        newState.EXMEM.instr = state.IDEX.instr;
        newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
        
        if (((state.IDEX.instr >> 22) & 7) == HALT ||
            ((state.IDEX.instr >> 22) & 7) == NOOP){
            EXMuxTop = state.IDEX.readRegA;
        }
        else{
            EXMuxTop = exstage_mux((state.IDEX.instr >> 19) & 7, state.IDEX.readRegA);
        }
        
        if (((state.IDEX.instr >> 22) & 7) == NAND ||
            ((state.IDEX.instr >> 22) & 7) == ADD ||
            ((state.IDEX.instr >> 22) & 7) == BEQ){
            EXMuxBottom = exstage_mux((state.IDEX.instr >> 16) & 7, state.IDEX.readRegB);
        }
        else{
            EXMuxBottom = state.IDEX.offset;
        }
        
        if (((state.IDEX.instr >> 22) & 7) == NAND){
            newState.EXMEM.aluResult = ~(EXMuxTop & EXMuxBottom); 
        }
        else if (((state.IDEX.instr >> 22) & 7) == BEQ){
            newState.EXMEM.aluResult = (EXMuxTop == EXMuxBottom); 
        }
        else{
            newState.EXMEM.aluResult = EXMuxTop + EXMuxBottom;
        }
        
        if (((state.IDEX.instr >> 22) & 7) == HALT ||
            ((state.IDEX.instr >> 22) & 7) == NOOP){
            newState.EXMEM.readRegB = state.IDEX.readRegB;
        }
        else{
            newState.EXMEM.readRegB = exstage_mux((state.IDEX.instr >> 16) & 7, state.IDEX.readRegB);
        }
        
        /* --------------------- MEM stage --------------------- */
        newState.MEMWB.instr = state.EXMEM.instr;
        if (((state.EXMEM.instr >> 22) & 7) == LW ||
            ((state.EXMEM.instr >> 22) & 7) == SW){
            newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
        }
        else{
            newState.MEMWB.writeData = state.EXMEM.aluResult;
        }

        if (((state.EXMEM.instr >> 22) & 7) == SW){
            newState.dataMem[state.EXMEM.aluResult] = memstage_mux((state.IDEX.instr >> 16) & 7, state.EXMEM.readRegB);
        }
        
        if (((state.EXMEM.instr >> 22) & 7) == BEQ){
            if (state.EXMEM.aluResult){
                newState.pc = state.EXMEM.branchTarget;
                newState.EXMEM.instr = NOOPINSTRUCTION;
                newState.IDEX.instr = NOOPINSTRUCTION;
                newState.IFID.instr = NOOPINSTRUCTION;
            }
        }
        
        /* --------------------- WB stage --------------------- */
        newState.WBEND.instr = state.MEMWB.instr;
        newState.WBEND.writeData = state.MEMWB.writeData;
        
        switch ((state.MEMWB.instr >> 22) & 7){
        case ADD: newState.reg[(state.MEMWB.instr & 7)] = state.MEMWB.writeData; break;
        case NAND: newState.reg[(state.MEMWB.instr & 7)] = state.MEMWB.writeData; break;
        case LW: newState.reg[((state.MEMWB.instr >> 16) & 7)] = state.MEMWB.writeData; break;
        }
        
        state = newState; /* this is the last statement before end of the loop.
                             It marks the end of the cycle and updates the
                             current state with the values calculated in this
                             cycle */
    }
}



