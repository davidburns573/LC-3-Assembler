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

int secondPass(void);


#endif
