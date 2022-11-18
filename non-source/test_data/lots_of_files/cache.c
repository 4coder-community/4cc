
/*
 * Instruction-level simulator for the LC
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum actionType
        {cacheToProcessor, processorToCache, memoryToCache, cacheToMemory,
         cacheToNowhere};
/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 * 	cacheToProcessor: reading data from the cache to the processor
 * 	processorToCache: writing data from the processor to the cache
 * 	memoryToCache: reading data from the memory to the cache
 * 	cacheToMemory: evicting cache data by writing it to the memory
 * 	cacheToNowhere: evicting cache data by throwing it away
 */
void
printAction(int address, int size, enum actionType type)
{
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);
    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    } else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    } else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    } else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    } else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

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
    int mem_[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct cacheStruct {
    int blockSizeInWords;
    int numberOfSets;
    int blocksPerSet;
    int *lruOrder;
    int *marks;
    
    int blockOffsetMask;
    int blockOffsetShift;
    int setIndexMask;
    int setIndexShift;
    int tagMask;
    int tagShift;
} cacheType;

#define MarkValid 0x80000000
#define MarkDirty 0x40000000
#define MarkTag   0x3FFFFFFF

void printState(stateType *);
void run(stateType, cacheType);
int convertNum(int);

int load(stateType *state, cacheType *cache, int addr);
void store(stateType *state, cacheType *cache, int addr, int data);

int log2(int x){
    int result = 0;
    int i;
    
    for (i = 1; i < 32; ++i){
        if ((x&1) == 1){
            result = i;
            break;
        }
        x >>= 1;
    }
    
    return (result);
}

int toint(char *s){
    int x;
    x = *(s++) - '0';
    for (;*s;){
        x *= 10;
        x += *(s++) - '0';
    }
    return(x);
}

#define SuppressState 1

int
main(int argc, char *argv[])
{
    int i, j, k;
    char line[MAXLINELENGTH];
    stateType state;
    cacheType cache;
    FILE *filePtr;

    if (argc != 5) {
        printf("error: usage: %s <machine-code file> blockSize numberOfSets blocksPerSet\n",
               argv[0]);
        exit(1);
    }

    cache.blockSizeInWords = toint(argv[2]);
    cache.numberOfSets = toint(argv[3]);
    cache.blocksPerSet = toint(argv[4]);

    cache.lruOrder = (int*)malloc(sizeof(int)*cache.numberOfSets*cache.blocksPerSet);
    cache.marks = (int*)malloc(sizeof(int)*cache.numberOfSets*cache.blocksPerSet);
    
    memset(cache.marks, 0, sizeof(int)*cache.numberOfSets*cache.blocksPerSet);

    i=0;
    for (j=0; j < cache.numberOfSets; ++j){
        for (k=0; k < cache.blocksPerSet; ++k, ++i){
            cache.lruOrder[i] = k;
        }
    }

    cache.blockOffsetShift = 0;
    cache.blockOffsetMask = (cache.blockSizeInWords - 1);
    cache.setIndexShift = log2(cache.blockSizeInWords) - 1;
    cache.setIndexMask = (cache.numberOfSets - 1);
    cache.tagShift = cache.setIndexShift + log2(cache.numberOfSets) - 1;
    cache.tagMask = ((~0) ^ (cache.blockOffsetMask) ^ (cache.setIndexMask)) >> cache.tagShift;
    
    /* initialize memories and registers */
    for (i=0; i<NUMMEMORY; i++) {
        state.mem_[i] = 0;
    }
    for (i=0; i<NUMREGS; i++) {
        state.reg[i] = 0;
    }
    
    state.pc=0;

    /* read machine-code file into instruction/data memory (starting at address 0) */

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
	if (sscanf(line, "%d", state.mem_+state.numMemory) != 1) {
	    printf("error in reading address %d\n", state.numMemory);
	    exit(1);
	}
	printf("memory[%d]=%d\n", state.numMemory, state.mem_[state.numMemory]);
    }

    printf("\n");
    
    /* run never returns */
    run(state, cache);

    return(0);
}

int computeAddress(cacheType *cache, int marks, int setIndex){
    int result;
    result = ((marks & MarkTag) << cache->tagShift) | (setIndex << cache->setIndexShift);
    return(result);
}

int evictAndAlloc(cacheType *cache, int setIndex){
    int *setMarks;
    int *lruOrder;
    int wayIndex;
    int baseAddress;

    lruOrder = cache->lruOrder + setIndex * cache->blocksPerSet;
    setMarks = cache->marks + setIndex * cache->blocksPerSet;
    wayIndex = lruOrder[0];
    baseAddress = computeAddress(cache, setMarks[wayIndex], setIndex);
    if (setMarks[wayIndex] & MarkValid){
        if (setMarks[wayIndex] & MarkDirty){
            printAction(baseAddress, cache->blockSizeInWords, cacheToMemory);
        }
        else{
            printAction(baseAddress, cache->blockSizeInWords, cacheToNowhere);
        }
    }

    return(wayIndex);
}

int lookup(cacheType *cache, int *setMarks, int tag, int *wayIndex){
    int hit, i;
    
    hit = 0;
    for (i=0; i < cache->blocksPerSet; ++i){
        if ((setMarks[i] & MarkValid) && (setMarks[i] & MarkTag) == tag){
            hit = 1;
            *wayIndex = i;
            break;
        }
    }

    return(hit);
}

int blockAddress(cacheType *cache, int addr){
    int result;
    result = addr & (~cache->blockOffsetMask);
    return(result);
}

void useLine(cacheType *cache, int setIndex, int wayIndex){
    int *lruOrder;
    int i;

    lruOrder = cache->lruOrder + setIndex * cache->blocksPerSet;

    for (i=0; i < cache->blocksPerSet; ++i){
        if (lruOrder[i] == wayIndex){
            memmove(lruOrder + i, lruOrder + i + 1, sizeof(int)*(cache->blocksPerSet - 1 - i));
            lruOrder[cache->blocksPerSet - 1] = wayIndex;
            break;
        }
    }
}

int load(stateType *state, cacheType *cache, int addr){
    int *setMarks;
    int setIndex, wayIndex;
    int tag;
    int hit, i;
    int baseAddress;

    setIndex = ((addr >> cache->setIndexShift) & cache->setIndexMask);
    tag = ((addr >> cache->tagShift) & cache->tagMask);
    setMarks = cache->marks + setIndex * cache->blocksPerSet;
    hit = lookup(cache, setMarks, tag, &wayIndex);
    
    if (!hit){
        wayIndex = evictAndAlloc(cache, setIndex);
        setMarks[wayIndex] = MarkValid | tag;
        baseAddress = blockAddress(cache, addr);
        printAction(baseAddress, cache->blockSizeInWords, memoryToCache);
    }

    useLine(cache, setIndex, wayIndex);
    printAction(addr, 1, cacheToProcessor);
    
    return(state->mem_[addr]);
}

void store(stateType *state, cacheType *cache, int addr, int data){
    int *setMarks;
    int setIndex, wayIndex;
    int tag;
    int hit, i;
    int baseAddress;

    setIndex = ((addr >> cache->setIndexShift) & cache->setIndexMask);
    tag = ((addr >> cache->tagShift) & cache->tagMask);
    setMarks = cache->marks + setIndex * cache->blocksPerSet;
    hit = lookup(cache, setMarks, tag, &wayIndex);

    if (!hit){
        wayIndex = evictAndAlloc(cache, setIndex);
        setMarks[wayIndex] = MarkValid | tag;
        baseAddress = blockAddress(cache, addr);
        printAction(baseAddress, cache->blockSizeInWords, memoryToCache);
    }

    setMarks[wayIndex] |= MarkDirty;
    printAction(addr, 1, processorToCache);

    useLine(cache, setIndex, wayIndex);    
    state->mem_[addr] = data;
}

void
run(stateType state, cacheType cache)
{
    int arg0, arg1, arg2, addressField;
    int instructions=0;
    int opcode;
    int maxMem=-1;	/* highest memory address touched during run */
    int instr;

    for (; 1; instructions++) { /* infinite loop, exits when it executes halt */
	printState(&state);

	if (state.pc < 0 || state.pc >= NUMMEMORY) {
	    printf("pc went out of the memory range\n");
	    exit(1);
	}

	maxMem = (state.pc > maxMem)?state.pc:maxMem;

	/* this is to make the following code easier to read */
    instr = load(&state, &cache, state.pc);
	opcode = instr >> 22;
	arg0 = (instr >> 19) & 0x7;
	arg1 = (instr >> 16) & 0x7;
	arg2 = instr & 0x7; /* only for add, nand */

	addressField = convertNum(instr & 0xFFFF); /* for beq, lw, sw */
	state.pc++;
	if (opcode == ADD) {
	    state.reg[arg2] = state.reg[arg0] + state.reg[arg1];
	} else if (opcode == NAND) {
	    state.reg[arg2] = ~(state.reg[arg0] & state.reg[arg1]);
	} else if (opcode == LW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
	    state.reg[arg1] = load(&state, &cache, state.reg[arg0] + addressField);
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
	} else if (opcode == SW) {
	    if (state.reg[arg0] + addressField < 0 ||
		    state.reg[arg0] + addressField >= NUMMEMORY) {
		printf("address out of bounds\n");
		exit(1);
	    }
        store(&state, &cache, state.reg[arg0] + addressField, state.reg[arg1]);
	    if (state.reg[arg0] + addressField > maxMem) {
		maxMem = state.reg[arg0] + addressField;
	    }
	} else if (opcode == BEQ) {
	    if (state.reg[arg0] == state.reg[arg1]) {
		state.pc += addressField;
	    }
	} else if (opcode == JALR) {
	    state.reg[arg1] = state.pc;
            if(arg0 != 0)
 		state.pc = state.reg[arg0];
	    else
		state.pc = 0;
	} else if (opcode == NOOP) {
	} else if (opcode == HALT) {
#ifndef SuppressState
	    printf("machine halted\n");
	    printf("total of %d instructions executed\n", instructions+1);
	    printf("final state of machine:\n");
#endif
	    printState(&state);
	    exit(0);
	} else {
	    printf("error: illegal opcode 0x%x\n", opcode);
	    exit(1);
	}
        state.reg[0] = 0;
    }
}

void
printState(stateType *statePtr)
{
#ifndef SuppressState
    int i;
    printf("\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
	for (i=0; i<statePtr->numMemory; i++) {
	    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem_[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
	}
    printf("end state\n");
#endif
}

int
convertNum(int num)
{
    /* convert a 16-bit number into a 32-bit Sun integer */
    if (num & (1<<15) ) {
	num -= (1<<16);
    }
    return(num);
}



