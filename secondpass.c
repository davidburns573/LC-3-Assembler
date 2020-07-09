#include "assembler.h"

/***global data***/
extern const char *INSTRUCTIONS[];
extern FILE *fptr;
extern struct totlines lines;
extern struct labeltable labels;
extern char conversionerror; //will be set to anything but zero if conversion error
extern short *mcode;



/**
 * check if line is a valid ADD instruction
 * @return 0 if found, -1 if error, -2 if not found
**/
int checkAdd(struct line *curline, int loc, short *code) {
    char *f = curline->chars;
    char *add = ADD;

    while (*f != ' ' && *f != '\t' && *f != '\0' && TOUPPER(*f) == *add) {
        if (TOUPPER(*f) < 'A' || TOUPPER(*f) > 'Z') {
            printf("Malformed instruction @x%04x\n", loc);
            return -1;
        }
        add++; f++;
    }
    if (*add != '\0') return -2;
    if (*add == '\0' && *f != ' ' && *f != '\t') {
        printf("Malformed ADD instruction @x%04x\n", loc);
        return -1;
    }

    *code = ADD_B;

    printf("ADD: %04x\n", *code);

    return 0;
}

/**
 * Loop through and test if line is equal to any instructions
 * @return 0 if normal, -1 if error
**/
int testInstructions(struct line *curline, int loc, short *code) {
    char *f = curline->chars;
    int sum = 0;
    while (*f != ' ' && *f != '\t' && *f != '\0') {
        sum += TOUPPER(*f);
        f++;
    }

    int instr = 0;
    if (sum == ADD_SUM)
        if ((instr = checkAdd(curline, loc, code)) != -2) return instr;
    
    return -2;
}

/**
 * Convert instructions into machine code
**/
int secondPass(void) {
    mcode = (short *) calloc(lines.size, sizeof(short));
    struct line *curline = lines.plines;
    short *code = mcode;
    int index = 0;

    int orig = checkOrig(curline->chars);
    if (orig == -1 || orig == -2) {
        printf("missing or malformed initial .orig\n");
    }
    *code = (short) orig;
    printf("BLOCK %x\n", *code);

    int i = 1;
    curline++; code++;
    while (i < lines.size) {
        int chOrig = checkOrig(curline->chars);
        if (chOrig == -1) return -1;
        else if (chOrig != -2) {
            orig = chOrig;
            index = 0;
            *code = (short) orig;
            printf("CODEBLOCK x%x\n", *code);
            i++; code++; curline++;
            continue;
        }

        if (testInstructions(curline, orig + index, code + i) == -1) return -1;

        index++; i++; code++; curline++;
    }
    return 0;
}

