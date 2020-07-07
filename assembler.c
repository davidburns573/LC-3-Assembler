#define  _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/***constants***/
#define ORIG ".orig"
#define END ".end"
#define INSTRSIZE 29
const char *INSTRUCTIONS[] = {"ADD","AND","BR","BRN","BRP","BRZ",
                              "BRNZ","BRNP","BRZP","BRNZP","GETC","HALT",
                              "IN","JMP","JSR","JSRR","LD","LDI",
                              "LDR","LEA","NOT","OUT","PUTS","RET",
                              "RTI","ST","STI","STR","TRAP"};

/***structs***/
struct line {
    int len;
    char *chars;
};

struct totlines {
    int size;
    struct line *plines;
};

struct label {
    char *name;
    int memlocation;
};

struct labeltable {
    int size;
    struct label *plabel;
};

/***global data***/
FILE *fptr;
struct totlines lines = {0, NULL};
struct labeltable labels = {0, NULL}; 
char conversionerror = 0; //will be set to anything but zero if conversion error
short *origtable = NULL;
short *mcode = NULL;


int openFile(char *filename) {
    fptr = fopen(filename, "r");
    if (!fptr) return 1;
    return 0;
}

/**
 * Allocates space for newline and adds it to lines
**/
void addLine(int len, char *newline) {
    lines.size = lines.size + 1;
    lines.plines = (struct line *) 
            realloc(lines.plines, lines.size * sizeof(struct line));

    struct line nline;
    nline.len = len;
    nline.chars = malloc(len);
    memcpy(nline.chars, newline, len);
    lines.plines[lines.size - 1] = nline;
}

/**
 * Removes all empty lines and comments
 * Puts file in more readable format using the line struct
**/
void parseFile(void) {
    size_t linecap = 0;
    char *newline = NULL;
    int linelen;
    while ((linelen = getline(&newline, &linecap, fptr)) != -1) {
        int i = 0;
        while (newline[i] == ' ' || newline[i] == '\t') i++;  //remove white space
                                        //remove comments and empty lines
        if (newline[i] != ';' && newline[i] != '\n' 
                && newline[i] != '\r' && newline[i] != '\0') {
                                        //remove newline
            if (newline[linelen - 2] == '\n' || newline[linelen - 2] == '\r') {
                newline[linelen - 2] = '\0';
                linelen--;
            }
            addLine(linelen - i, (newline + i));
        }
    }

    free(newline);
}

/***recursive pow function (may not use)***/
int powfunc(int a, int b) {
    if (b == 0) {
        return 1;
    } else if (b == 1) {
        return a;
    }
    if (b & 1) {
        return a * powfunc(a, b - 1);
    } else {
        int i = powfunc(a, b / 2);
        return i * i;
    }
}

int octalStrToInt(char *f) {
    f++; //remove inital 0 to denote an octal number
    int sum = 0;
    char *s = f;
    int size = 0;
    while (*f != '\0' && *f != ' ' && *f != '\t') {
        if (*f < '0' || *f > '7' || size > 10) {
            conversionerror = 1;
            return -1;
        }
        size++; f++;
    }
    while (*f != '\0') {
        if (*f != ' ' && *f != '\t') {
            conversionerror = 1;
            return -1;
        }
        f++;
    }
    while (size >= 0) {
        size--;
        sum = sum | ((*s - '0') << (size * 3));
        s++;
    }
    return sum;
}

int hexStrToInt(char *f) {
    f++; //remove inital x to denote a hex number
    int sum = 0;
    char *s = f;
    int size = 0;
    while (*f != '\0' && *f != ' ' && *f != '\t') {
        if ((*f < '0' || (*f > '9' && *f < 'A') || (*f > 'F' && *f < 'a')
                || (*f > 'f')) || size > 10) {
            conversionerror = 1;
            return -1;
        }
        size++; f++;
    }
    while (*f != '\0') {
        if (*f != ' ' && *f != '\t') {
            conversionerror = 1;
            return -1;
        }
        f++;
    }
    while (size >= 0) {
        size--;
        int n;
        if (*s <= '9') n = *s - '0';
        else if (*s <= 'F') n = *s - 'A' + 10;
        else n = *s - 'a' + 10;

        sum = sum | (n << (size * 4));
        s++;
    }
    return sum;
}

int decStrToInt(char *f) {
    if (*f == '#') f++; //remove inital # to denote a decimal number
    int sum = 0;
    char *s = f;
    int size = 0;
    int pow = 1;
    while (*f != '\0' && *f != ' ' && *f != '\t') {
        if (*f < '0' || *f > '9' || size > 10) {
            conversionerror = 1;
            return -1;
        }
        pow *= 10;
        size++; f++;
    }
    while (*f != '\0') {
        if (*f != ' ' && *f != '\t') {
            conversionerror = 1;
            return -1;
        }
        f++;
    }
    pow /= 10;
    while (size > 0) {
        sum += ((*s - '0') * pow);
        pow /= 10;
        s++; size--;
    }
    return sum;
    return 0;
}

/**
 * Checks for a .orig statement
 * @param f a line
 * @return location of orig, -1 if malformed, -2 if not there
**/
int checkOrig(char *f) {
    char *orig = ORIG;
    while (*orig != '\0') {
        if (*f != *orig) {
            return -2;
        }
        orig++; f++;
    }   //ensure space between .orig and number
    if (*f != ' ') {
        printf("malformed .orig statement\n"); 
        return -1;
    }

    while (*f == ' ' || *f == '\t') f++;

    int codestart = 0;
    switch (*f) {
        case '0':
            codestart = octalStrToInt(f);
            break;
        case 'x': case 'X':
            codestart = hexStrToInt(f);
            break;
        default:
            codestart = decStrToInt(f);
            break;
    }

    if (conversionerror) {
        printf(".orig not a valid number\n");
        return -1;
    } else if (codestart < 0) return -1;
    return codestart;
}

/**
 * Checks for a .end statement
 * @param f a line
 * @return 0 if normal, -1 if malformed, -2 if not there
**/
int checkEnd(char *f) {
    char *end = END;
    while (*end != '\0') {
        if (*f != *end) {
            return -2;
        }
        end++; f++;
    }
    while (*f != '\0') {
        if (*f != ' ' && *f != '\t') {
            printf("malformed .end statement\n");
            return -1;
        }
    }
    return 0;
}

/**
 * Checks if given line is a label
**/
int checkForLabel(char *f) {
    char *p = f;
    while (*p != '\0' && *p != ' ' && *p != '\t') { //checks first word, only letters
        if ((*p & ~(1 << 5)) < 'A' || (*p & ~(1 << 5)) > 'Z') return 0;
        p++;
    }
    for (int i = 0; i < INSTRSIZE; i++) {
        char *instr = INSTRUCTIONS[i];
        char *x = f;
        while (*instr != '\0' && (*instr == (*x & ~(1 << 5)))) {
            if ((*x & ~(1 << 5)) < 'A' || (*x & ~(1 << 5)) > 'Z') return 0; //remove PSEUDO-OPs
            instr++; x++;
        }
        if (*instr == '\0' && (*x == ' ' || *x == '\t')) return 0; //return 0 for not label
        char up = (*f & ~(1 << 5));
        if (up > 'A' && up < 'Z' && up < *INSTRUCTIONS[i]) return 1; //return 1 for label
    }
    return 1;
}

/**
 * frees label table
**/
void freeLabelTable(void) {
    for (int i = 0; i < labels.size; i++) {
        free((labels.plabel + i)->name);
    }
    free(labels.plabel);
    labels.plabel = NULL;
    labels.size = 0;
}

/**
 * remove empty instruction after removing label
**/
void removeEmptyInstruction(void) {
    struct line *curline = lines.plines;
    int i = 0;
    while (i < lines.size && curline->chars != NULL) {
        i++; curline++;
    }
    printf("size: %d inc: %d\n", lines.size, i);
    while (i < lines.size - 1) {
        *curline = *(curline + 1);
        curline++; i++;
    }
    lines.size += -1;
}

/**
 * removes label from instruction
 * @char *f start of line string immediately after label
**/
int removeLabel(struct line *curline, char *f) {
    while (*f == ' ' || *f == '\t') f++;
    if (*f == '\0') {
        curline->len = 0;
        free(curline->chars);
        curline->chars = NULL;
        removeEmptyInstruction();
        return 1;
    }
    char *c = f;
    int numLetters = 0;
    while (*c != '\0') {
        c++; numLetters++;
    }
    numLetters++;
    c = (char *) malloc(numLetters);
    memcpy(c, f, numLetters);
    c[numLetters - 1] = '\0';

    free(curline->chars);
    curline->chars = c;
    curline->len = numLetters;
    
    return 0;
}

/**
 * Adds label to label table
**/
int addLabel(struct line *curline, int location) {
    char *c = curline->chars;
    int numLetters = 0;
    while (*c != '\0' && *c != ' ' && *c != '\t') {
        c++; numLetters++;
    }
    char *end = c;
    c = (char *) malloc(numLetters + 1);
    memcpy(c, curline->chars, numLetters);
    c[numLetters] = '\0';

    struct label nlabel;
    nlabel.memlocation = location;
    nlabel.name = c;
    
    labels.size = labels.size + 1;
    labels.plabel = (struct label *) realloc(labels.plabel, 
                        labels.size * sizeof(struct label));
    labels.plabel[labels.size - 1] = nlabel;

    return removeLabel(curline, end);
}

/**
 * Create label table with locations in code 
**/
int firstPass(void) {
    origtable = (short *) malloc(sizeof(short) * lines.size);
    short *origs = origtable;
    char *first = lines.plines->chars;
    int orig = checkOrig(first); //-1 if missing, -2 if malformed
    if (orig == -2) {
        printf("Missing .orig statement\n");
        return -1;
    } else if (orig == -1) return -1;
    *origs = *origs | orig;
    origs++;

    struct line *curline = lines.plines;
    curline++;
    int inc = 0;
    int end = lines.size - 1;
    while (inc < end - 1) {
        int checkend = 0;
        if ((checkend = checkEnd(curline->chars)) == 0) {    //.end is there
            orig = -1;
            goto NEXT;
        } else if (checkend == -1) return -1; //.end is malformed
        int chOrig = 0;

        if ((chOrig = checkOrig(curline->chars)) >= 0) {
            if (orig == -1) {
                end = end - inc;
                inc = 0;
                orig = chOrig - 1;
                *origs = *origs | chOrig;
                goto NEXT;
            } else {
                printf(".orig statement before .end statement");
                return -1;
            }
        } else if (chOrig == -2 && orig == -1) {
            printf("Missing .orig statement\n");
            return -1; 
        } else if (chOrig == -1) return -1;

        if (checkForLabel(curline->chars)) {
            if (addLabel(curline, inc + orig)) {
                end += -1;
                continue;
            }
        }

        NEXT:
        inc++; curline++; origs++;
    }

    int checkFinalEnd = 0;
    if ((checkFinalEnd = checkEnd(curline->chars)) == -2) {
        printf("Missing .end statement\n" );
        return -1;
    } else if (checkFinalEnd == -1) return -1;

    return 0;
}

/**
 * Convert instructions into machine code
**/
int secondPass(void) {
    struct line *f = lines.plines;
    short *code = mcode;
    int orig = checkOrig(f->chars);
    int size = lines.size - 1;
    int inc = 0;
    while (inc < size) {
        

        inc++;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Need file to assemble\n"); 
        return 1;
    }

    if (openFile(argv[1])) {
        printf("Error opening file\n"); 
        return 1;
    }

    parseFile();

    if (firstPass() == -1) return 1;

    for (int i = 0; i < labels.size; i++) {
        printf("loc: %X name: %s\n", (labels.plabel + i)->memlocation,(labels.plabel + i)->name);
    }

    for (int i = 0; i < lines.size && (lines.plines + i)->chars != NULL; i++) {
        printf("%s\n", (lines.plines + i)->chars);
    }

    mcode = (short *) malloc(sizeof(short *) * lines.size);
    if (secondPass() == -1) return 1;

    freeLabelTable();
    return 0;
}