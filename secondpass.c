#include "assembler.h"

/***global data***/
extern const char *INSTRUCTIONS[];
extern FILE *fptr;
extern struct totlines lines;
extern struct labeltable labels;
extern char conversionerror; //will be set to anything but zero if conversion error
extern short *origtable;
extern short *mcode;



/**
 * Convert instructions into machine code
**/
int secondPass(void) {
    struct line *f = lines.plines;
    short *code = mcode;
    int size = lines.size - 1;
    int inc = 0;
    while (inc < size) {
        

        inc++;
    }
    return 0;
}

