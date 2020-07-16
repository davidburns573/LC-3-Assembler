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
#define BR "BR"
#define BR_B (0 << 12)

/***sums***/
#define ADD_SUM ('A' + 'D' + 'D')
#define AND_SUM ('A' + 'N' + 'D')
#define NOT_SUM ('N' + 'O' + 'T')
#define JMP_SUM ('J' + 'M' + 'P')
#define BR_SUM ('B' + 'R')
#define BRNZP_SUM ('B' + 'R' + 'N' + 'Z' + 'P')

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
