#include "assembler.h"

/***global data***/
const char *INSTRUCTIONS[] = {"ADD","AND","BR","BRN","BRP","BRZ",
                              "BRNZ","BRNP","BRZP","BRNZP","GETC","HALT",
                              "IN","JMP","JSR","JSRR","LD","LDI",
                              "LDR","LEA","NOT","OUT","PUTS","PUTSP","RET",
                              "RTI","ST","STI","STR","TRAP"};
FILE *fptr;
struct totlines lines = {0, NULL};
struct labeltable labels = {0, NULL}; 
char conversionerror = 0; //will be set to anything but zero if conversion error
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
    int negative = 0;
    if (*f == '-') {    //negative == 1 if '-' present
        negative = 1;
        f++;
    }

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
    if (negative) return -sum;
    return sum;
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
        printf("loc: x%X name: %s\n", (labels.plabel + i)->memlocation,(labels.plabel + i)->name);
    }

    // for (int i = 0; i < lines.size; i++) {
    //     printf("%d %s\n", i, (lines.plines + i)->chars);
    // }

    printf("\n-------------------\n\n");

    if (secondPass() == -1) return 1;

    // for (int i = 0; i < lines.size; i++) {
    //     printf("%d x%04hX\n", i, *(mcode + i));
    // }

    freeLabelTable();
    return 0;
}