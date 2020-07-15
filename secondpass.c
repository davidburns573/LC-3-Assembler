#include "assembler.h"

/***global data***/
extern const char *INSTRUCTIONS[];
extern FILE *fptr;
extern struct totlines lines;
extern struct labeltable labels;
extern char conversionerror; //will be set to anything but zero if conversion error
extern short *mcode;

enum br_enum{
    NZP,NZ,NP,ZP,N,Z,P,NOTSET
} BR_enum;

char *nzpArr[7] = {"NZP","NZ","NP","ZP","N","Z","P"};
const int nzpSize = 7;


/**
 * parse 1 register given a character string
 * @return NULL if error, updated pointer if not
**/
char * parseReg(char *f, int *val) {
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
char * parse2Reg(char *f, int *first, int *second) {
    if ((f = parseReg(f, first)) == NULL) return NULL;

    while (*f == ' ' || *f == '\t') f++;
    if (*f != ',') return NULL;
    f++;
    
    if ((f = parseReg(f, second)) == NULL) return NULL;

    return f;
}

/**
 * parse offset value
 * @returns updated pointer, NULL otherwise
**/
char * parseOffset(char *f, int *val) {
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

    if (conversionerror) return NULL;

    while (*f != '\0' && *f != ' ' && *f != '\t' && *f != ';') f++;
    
    return f;
}

/**
 * Checks if instr is equal to passed in character string
 * @returns indexed pointer if valid, NULL if error, and -2 if not equal
**/
char * checkInstruction(char *instr, char *f) {
    while (*instr != '\0' && *f != ' ' && *f != '\t' 
            && *f != '\0' && TOUPPER(*f) == *instr) {
        if (TOUPPER(*f) < 'A' || TOUPPER(*f) > 'Z') return NULL;
        instr++; f++;
    }
    if (*instr != '\0') return (char *) -2;
    if (*instr == '\0' && *f != ' ' && *f != '\t' && *f != '\0') return NULL;
    return f;
}

/**
 * check if line is a valid ADD/AND instruction
 * @return 0 if found, -1 if error, -2 if not found
**/
int checkAddAnd(struct line *curline, short *code, char *instr) {
    char *f = curline->chars;
    char *stinstr = instr;

    if ((f = checkInstruction(instr, f)) == NULL) return -1;
    else if ((long) f == -2) return -2;

    if (*(stinstr+1) == 'D') *code = ADD_B;
    else *code = AND_B;

    int dst, src1;
    if ((f = parse2Reg(f, &dst, &src1)) == NULL) {

    }
    *code = (*code) | ((7 & dst) << 9) | ((7 & src1) << 6);
    
    while (*f == ' ' || *f == '\t') f++;
    if (*f != ',') return -1;

    f++;
    while (*f == ' ' || *f == '\t') f++;

    int val = 0;
    if (TOUPPER(*f) == 'R') {
        if ((f = parseReg(f, &val)) == NULL) return -1;
        *code = (*code) | (7 & val);
    } else {
        if ((f = parseOffset(f, &val)) == NULL) return -1;
        if (val < -16 || val > 15) return -1;
        *code = (*code) | (31 & val) | (1 << 5);
    }

    while (*f != '\0') {
        if (*f == ';') return 0;
        if (*f != ' ' && *f != '\t') return -1;
        f++;
    }

    return 0;
}

/**
 * check if line is a valid BR instruction
 * @return 0 if found, -1 if error, -2 if not found
**/
int checkBr(struct line *curline, int loc, short *code) {
    char *f = curline->chars;
    char *br = BR;

    while (*br != '\0' && *f != '\0' && *br == TOUPPER(*f)) {
        f++; br++;
    }
    if (*br != '\0') return -2;

    if (*f == ' ') BR_enum = NZP;
    else BR_enum = NOTSET;

    char *stf = f;
    for (int i = 0; i < nzpSize && BR_enum == NOTSET; i++) {
        if ((f = checkInstruction(nzpArr[i], stf)) == (char *) -1) return -1;
        else if (f != (char *) -2) BR_enum = i;
    }
    if (f == (char *) -2) return -1;

    char n = 0;
    char z = 0;
    char p = 0;
    switch (BR_enum) {
        case NZP: n = 1; z = 1; p = 1; break;
        case NZ: n = 1; z = 1; break;
        case NP: n = 1; p = 1; break;
        case ZP: z = 1; p = 1; break;
        case N: n = 1; break;
        case Z: z = 1; break;
        case P: p = 1; break;
        default: return -1;
    }
    *code = *code | (n << 11) | (z << 10) | (p << 9) | BR_B;

    while (*f == ' ' || *f == '\t') f++;
    if (TOUPPER(*f) > 'Z' || *f < '0' || 
        (TOUPPER(*f) < 'A' && *f > '9') || *f == '#') return -1;

    int val = 0;
    char *cf = f;
    if ((cf = parseOffset(f, &val)) != NULL) {
        if (val > 255 || val < -256) {
            printf("Offset value doesn't fit in 9 bits\n");
            return -1;
        }
        *code = *code | (511 & val);
    } else {
        conversionerror = 0;
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
        if ((instr = checkAddAnd(curline, code, ADD)) != -2) return instr;

    if (sum == AND_SUM)
        if ((instr = checkAddAnd(curline, code, AND)) != -2) return instr;
    
    if (sum >= BR_SUM && sum <= BRNZP_SUM) 
        if ((instr = checkBr(curline, loc, code)) != -2) return instr;

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

        if (testInstructions(curline, orig + index, code) == -1) {
            printf("Malformed instruction @x%04x\n", orig + index);
            return -1;
        }

        index++; i++; code++; curline++;
    }
    return 0;
}

