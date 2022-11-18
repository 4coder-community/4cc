/* Assembler code fragment for LC-2K */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *, int *);

typedef struct{
    char *word;
    unsigned int hash;
    int line_number;
} Label;

typedef struct{
    char *word_space;
    char *word_cursor;
    Label *labels;
    int count;
    int max;
} Table;

char *
copy_advance(char *dest, char *src){
    while (*dest++ = *src++);
    return(dest);
}

int
get_hash(char *string){
    unsigned int x, c;
    x = 5236;
    while (c = *string++){
        x += c + (x << 5) + (x << 1);
    }
    return(x);
}

int
add_label(Table *table, char *string, int line_number){
    int r;
    Label l, *ls;
    int max;
    unsigned int index;
    
    r = 0;
    max = table->max;
    ls = table->labels;
    
    l.word = table->word_cursor;
    table->word_cursor = copy_advance(table->word_cursor, string);
    l.hash = get_hash(string);
    l.line_number = line_number;
    index = l.hash % max;
    
    while (ls[index].word){
        if (ls[index].hash == l.hash){
            if (strcmp(ls[index].word, l.word) == 0){
                r = 1;
                break;
            }
        }
        index = (index + 1) % max;
    }
    
    if (r == 0) ls[index] = l;
    ++table->count;
    
    return(r);
}

int
find_label(Table *table, char *string, int *line_number){
    int r;
    Label l, *ls;
    int max;
    unsigned int hash,index;
    
    r = 0;
    max = table->max;
    ls = table->labels;
    hash = get_hash(string);
    index = hash % max;
    
    while (ls[index].word){
        if (ls[index].hash == hash){
            if (strcmp(ls[index].word, string) == 0){
                r = 1;
                *line_number = ls[index].line_number;
                break;
            }
        }
        index = (index + 1) % max;
    }
    
    return(r);
}

#define OP_FILL -1
#define OP_ADD 0
#define OP_NAND 1
#define OP_LW 2
#define OP_SW 3
#define OP_BEQ 4
#define OP_JALR 5
#define OP_HALT 6
#define OP_NOOP 7

#define MIN16S -32768
#define MAX16S 32767

int
encode_insanity(FILE *outFilePtr, int o, int a, int b, int c){
    int ins, r;
    
    r = 0;
    if (o == OP_FILL){
        r = (b != 0 || c != 0);
    }
    else{
        r = (a < 0 || a > 7 || b < 0 || b > 7);
        if (!r){
            switch (o){
            case OP_ADD: case OP_NAND:
                r = (c < 0 || c > 7); break;
                
            case OP_LW: case OP_SW:
            case OP_BEQ: case OP_JALR:
                r = (c < MIN16S || c > MAX16S); break;
                
            case OP_HALT: case OP_NOOP:
                r = (a != 0 || b != 0 || c != 0); break;
                
            default: r = 1; break;
            }
        }
    }
    
    if (!r){
        if (o == OP_FILL){
            ins = a;
        }
        else{
            ins = 0;
            ins |= (o << 22);
            ins |= (a << 19);
            ins |= (b << 16);
            ins |= 0xFFFF & (c);
        }
        fprintf(outFilePtr, "%d\n", ins);
    }
    
    return(r);
}

int
main(int argc, char *argv[])
{
    Table table;
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];
    int max_instructions;
    int line_number;
    int i;
    int o,a,b,c;

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }
    
    fseek(inFilePtr, 0, SEEK_END);
    max_instructions = ftell(inFilePtr);
    fseek(inFilePtr, 0, SEEK_SET);
    max_instructions >>= 2;
    
    table.max = max_instructions * 3 / 2;
    table.labels = (Label*)malloc(sizeof(Label)*table.max);
    table.count = 0;
    table.word_space = table.word_cursor = (char*)malloc((max_instructions << 2) + max_instructions);
    memset(table.labels, 0, sizeof(Label)*table.max);
    
    line_number = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        if (*label != 0){
            if (add_label(&table, label, line_number)){
                printf("error two labels of same name: %s\n", label);
                exit(1);
            }
        }
        ++line_number;
    }
    
    rewind(inFilePtr);
    
    line_number = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        if (strcmp(opcode, "add") == 0){
            o = 0;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            sscanf(arg2, "%d", &c);
        }
        else if (strcmp(opcode, "nand") == 0){
            o = 1;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            sscanf(arg2, "%d", &c);
        }
        else if (strcmp(opcode, "lw") == 0){
            o = 2;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            if (!isNumber(arg2, &c)){
                if (!find_label(&table, arg2, &c)){
                    printf("non-existant label: %s\n", arg2);
                    exit(1);
                }
            }
        }
        else if (strcmp(opcode, "sw") == 0){
            o = 3;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            if (!isNumber(arg2, &c)){
                if (!find_label(&table, arg2, &c)){
                    printf("non-existant label: %s\n", arg2);
                    exit(1);
                }
            }
        }
        else if (strcmp(opcode, "beq") == 0){
            o = 4;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            if (!isNumber(arg2, &c)){
                if (!find_label(&table, arg2, &c)){
                    printf("non-existant label: %s\n", arg2);
                    exit(1);
                }
                c = c - (line_number + 1);
            }
        }
        else if (strcmp(opcode, "jalr") == 0){
            o = 5;
            sscanf(arg0, "%d", &a);
            sscanf(arg1, "%d", &b);
            c = 0;
        }
        else if (strcmp(opcode, "halt") == 0){
            o = 6;
            a = b = c = 0;
        }
        else if (strcmp(opcode, "noop") == 0){
            o = 7;
            a = b = c = 0;
        }
        else if (strcmp(opcode, ".fill") == 0){
            o = -1;
            if (!isNumber(arg0, &a)){
                if (!find_label(&table, arg0, &a)){
                    printf("non-existant label: %s\n", arg0);
                    exit(1);
                }
            }
            b = c = 0;
        }
        else{
            printf("not real instruction found: %s\n", opcode);
            exit(1);
        }
        
        if (encode_insanity(outFilePtr, o, a, b, c)){
            printf("encoding error on line %d\n", line_number + 1);
            exit(1);
        }
        
        ++line_number;
    }
    
    free(table.word_space);
    free(table.labels);
    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(0);
    
    return(0);
}

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH + 1];
    char *ptr = line;
    int l;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }
    
    if (feof(inFilePtr)){
        l = strlen(line);
        if (line[l-1] != '\n'){
            line[l] = '\n';
            line[l+1] = 0;
        }
    }
    
#if 0
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }
#endif

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }
    
    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string, int *i)
{
    return( (sscanf(string, "%d", i)) == 1);
}
