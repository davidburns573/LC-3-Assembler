#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#define  _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/***macros***/
#define TOLOWER(a) ((a >= 'A' && a <= 'Z') ? (a) : ((a) | (1 << 5)))
#define TOUPPER(a) ((a) & ~(1 << 5))
#define ORIG ".orig"
#define END ".end"
#define STRINGZ ".stringz"
#define BLKW ".blkw"
#define FILL ".fill"
#define INSTRSIZE 29

/***instructions***/
#define ADD "ADD"
#define ADD_B (1 << 12)
#define AND "AND"
#define AND_B (5 << 12)
#define NOT "NOT"
#define NOT_B ((9 << 12) | (127))
#define JMP "JMP"
#define JMP_B (12 << 12)
#define JSR "JSR"
#define JSR_B ((4 << 12) | (1 << 11))
#define JSRR "JSRR"
#define JSRR_B (4 << 12)
#define LD "LD"
#define LD_B (2 << 12)
#define LDI "LDI"
#define LDI_B (10 << 12)
#define LDR "LDR"
#define LDR_B (6 << 12)
#define ST "ST"
#define ST_B (3 << 12)
#define STI "STI"
#define STI_B (11 << 12)
#define STR "STR"
#define STR_B (7 << 12)
#define LEA "LEA"
#define LEA_B (13 << 12)
#define BR "BR"
#define BR_B (0 << 12)
#define RET "RET"
#define RTI "RTI"
#define RTI_B (8 << 12)

/***sums***/
#define ADD_SUM ('A' + 'D' + 'D')
#define AND_SUM ('A' + 'N' + 'D')
#define NOT_SUM ('N' + 'O' + 'T')
#define JMP_SUM ('J' + 'M' + 'P')
#define JSR_SUM ('J' + 'S' + 'R')
#define JSRR_SUM ('J' + 'S' + 'R' + 'R')
#define LD_SUM ('L' + 'D')
#define LDI_SUM ('L' + 'D' + 'I')
#define LDR_SUM ('L' + 'D' + 'R')
#define ST_SUM ('S' + 'T')
#define STI_SUM ('S' + 'T' + 'I')
#define STR_SUM ('S' + 'T' + 'R')
#define LEA_SUM ('L' + 'E' + 'A')
#define BR_SUM ('B' + 'R')
#define BRNZP_SUM ('B' + 'R' + 'N' + 'Z' + 'P')
#define RET_SUM ('R' + 'E' + 'T')
#define RTI_SUM ('R' + 'T' + 'I')

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

/***functions***/
int octalStrToInt(char *f);
int hexStrToInt(char *f);
int decStrToInt(char *f);

int firstPass(void);
int checkOrig(char *f);

int secondPass(void);


#endif
