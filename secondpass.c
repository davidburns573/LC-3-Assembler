#include "assembler.h"

/***global data***/
extern const char *INSTRUCTIONS[];
extern FILE *fptr;
extern struct totlines lines;
extern struct labeltable labels;
extern char conversionerror; //will be set to anything but zero if conversion error
extern short *mcode;


/**
 * parse 1 register given a character string
 * @return NULL if error, updated pointer if not
**/
char * parseReg(char *f, char *val) {
    while (*f == ' ' || *f == '\t') f++;
    if (TOUPPER(*f) != 'R') return NULL;
    f++;
    if (*f < '0' || *f > '7') return NULL;
    *val = (*f - '0');
    f++;
    return f;
}

/**
 * parse 2 registers given a character string
 * A char (bits 0-3 are second reg, 4-7 are first reg), -1 if error
**/
char * parse2Reg(char *f, char *first, char *second) {
    if ((f = parseReg(f, first)) == NULL) return NULL;

    while (*f == ' ' || *f == '\t') f++;
    if (*f != ',') return NULL;
    f++;
    
    if ((f = parseReg(f, second)) == NULL) return NULL;

    return f;
}

char * parseImmediate(char *f, char *val) {
    while (*f == ' ' || *f == '\t') f++;

    switch (*f) {
        case '0':
            *val = octalStrToInt(f);
            break;
        case 'x': case 'X':
            *val = hexStrToInt(f);
            break;
        default:
            *val = decStrToInt(f);
            break;
    }

    printf("%d\n", *val);

    if (conversionerror) return NULL;
    if (*val < -16 || *val > 15) return NULL;

    while (*f != '\0' && *f != ' ' && *f != '\t' && *f != ';') f++;
    
    return f;
}

/**
 * check if line is a valid ADD/AND instruction
 * @return 0 if found, -1 if error, -2 if not found
**/
int checkAddAnd(struct line *curline, int loc, short *code, char *instr) {
    char *f = curline->chars;
    char *stinstr = instr;

    while (*f != ' ' && *f != '\t' && *f != '\0' && TOUPPER(*f) == *instr) {
        if (TOUPPER(*f) < 'A' || TOUPPER(*f) > 'Z') {
            printf("Malformed instruction @x%04x\n", loc);
            return -1;
        }
        instr++; f++;
    }
    if (*instr != '\0') return -2;
    if (*instr == '\0' && *f != ' ' && *f != '\t') {
        printf("Malformed instruction @x%04x\n", loc);
        return -1;
    }

    if (*(stinstr+1) == 'D') *code = ADD_B;
    else *code = AND_B;

    char dst, src1;
    if ((f = parse2Reg(f, &dst, &src1)) == NULL) {

    }
    *code = (*code) | ((7 & dst) << 9) | ((7 & src1) << 6);
    
    while (*f == ' ' || *f == '\t') f++;
    if (*f != ',') {
        printf("Malformed instruction @x%04x\n", loc);
        return -1;
    }
    f++;
    while (*f == ' ' || *f == '\t') f++;

    char val = 0;
    if (TOUPPER(*f) == 'R') {
        if ((f = parseReg(f, &val)) == NULL) {
            printf("Malformed instruction @x%04x\n", loc);
            return -1;
        }
        *code = (*code) | (7 & val);
    } else {
        if ((f = parseImmediate(f, &val)) == NULL) {
            printf("Malformed instruction @x%04x\n", loc);
            return -1;
        }
        *code = (*code) | (31 & val) | (1 << 5);
    }

    while (*f != '\0') {
        if (*f == ';') return 0;
        if (*f != ' ' && *f != '\t') {
            printf("Malformed instruction @x%04x\n", loc);
            return -1;
        }
        f++;
    }

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
        if ((instr = checkAddAnd(curline, loc, code, ADD)) != -2) return instr;

    if (sum == AND_SUM)
        if ((instr = checkAddAnd(curline, loc, code, AND)) != -2) return instr;
    
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

        if (testInstructions(curline, orig + index, code) == -1) return -1;

        index++; i++; code++; curline++;
    }
    return 0;
}

