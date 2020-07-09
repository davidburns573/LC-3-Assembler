#include "assembler.h"

/***global data***/
extern const char *INSTRUCTIONS[];
extern FILE *fptr;
extern struct totlines lines;
extern struct labeltable labels;
extern char conversionerror; //will be set to anything but zero if conversion error
extern short *mcode;

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
 * remove empty instruction after removing label
**/
void removeEmptyInstruction(void) {
    struct line *curline = lines.plines;
    int i = 0;
    while (i < lines.size && curline->chars != NULL) {
        i++; curline++;
    }
    while (i < lines.size - 1) {
        *curline = *(curline + 1);
        curline++; i++;
    }
    lines.size += -1;
    //resize lines.plines to - 1 of original size
    lines.plines = (struct line *) realloc(lines.plines, 
                            sizeof(struct line) * lines.size);
}

/**
 * Checks for a .end statement
 * @param f a line
 * @return 0 if there, -1 if malformed, -2 if not there
**/
int checkEnd(struct line *curline) {
    char *f = curline->chars;
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
    free(curline->chars);
    curline->chars = NULL;
    removeEmptyInstruction();
    return 0;
}

/**
 * Checks if given line is a label
**/
int checkForLabel(char *f) {
    char *p = f;
    while (*p != '\0' && *p != ' ' && *p != '\t') { //checks first word, only letters
        if (TOUPPER(*p) < 'A' || TOUPPER(*p) > 'Z') return 0;
        p++;
    }
    for (int i = 0; i < INSTRSIZE; i++) {
        char *instr = (char *) INSTRUCTIONS[i];
        char *x = f;
        while (*instr != '\0' && (*instr == TOUPPER(*x))) {
            if ((*x & ~(1 << 5)) < 'A' || TOUPPER(*x) > 'Z') return 0; //remove PSEUDO-OPs
            instr++; x++;
        }
        if (*instr == '\0' && (*x == ' ' || *x == '\t')) return 0; //return 0 for not label
        char up = TOUPPER(*f);
        if (up > 'A' && up < 'Z' && up < *INSTRUCTIONS[i]) return 1; //return 1 for label
    }
    return 1;
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
 * Adds @size number of lines and converts @index - @index + @size
 * into empty comment lines that can easily be parsed ("$0000")
**/
void addLines(int index, int size) {
    lines.size = lines.size + size;
    lines.plines = (struct line *) realloc(lines.plines, 
                                sizeof(struct line) * lines.size);
    char *c = (char *) malloc(6);
    strcpy(c, "$0000");
    struct line empty;
    empty.len = 6;
    empty.chars = c;
    for (int i = 0; i < size; i++) { //initialize extra lines to the empty line
        lines.plines[lines.size - size + i] = empty;
    }
    int end = lines.size - size - 1;
    while (end > index) {
        lines.plines[end + size] = lines.plines[end];
        lines.plines[end] = empty;
        end--;
    }
    free(lines.plines[index].chars);
    lines.plines[index] = empty;
}

/**
 * parses .stringz to ensure the pseudo-op is valid and determines
 * the size of the string
**/
int parseStringz(char *f) {
    if (*f != '"') {
        printf(".stringz invalid string\n");
        return -1;
    }
    f++;
    int size = 1;
    while (*f != '"') {
        if (*f == '\0') {
            printf(".stringz invalid string\n");
            return -1;
        }
        f++; size++;
    }
    f++;
    while (*f != '\0') {
        if (*f != ' ' && *f != '\t') {
            printf("malformed .stringz pseudo-op\n");
            return -1;
        }
        f++;
    }
    return size;
}

/**
 * Parses .blkw line and finds number of lines to add
**/
int parseBlkw(char *f) {
    int size = 0;
    switch(*f) {
        case '0' :
            size = octalStrToInt(f);
            break;
        case 'x': case 'X':
            size = hexStrToInt(f);
            break;
        default:
            size = decStrToInt(f);
            break;
    }
    if (conversionerror) {
        printf(".blkw not a valid number\n");
        return -1;
    } else if (size <= 0) {
        printf(".blkw cannot be zero or negative\n");
        return -1;
    }
    return size;
}

void shortToHexStr(char *f, short s) {
    for (int i = 3; i >= 0; i--) {
        short x = 15 & (s >> (i * 4));
        if (x <= 9) {
            *f = x + '0';
        } else {
            *f = x + 'A' - 10;
        }
        f++;
    }
}

void addString(int index, char *f) {
    struct line *p = &lines.plines[index];
    f++; //remove quotation mark
    while (*f != '"') {
        char *c = (char *) malloc(6);
        *c = '$';
        shortToHexStr(c + 1, *f);
        c[5] = '\0';
        struct line nline;
        nline.len = 6;
        nline.chars = c;
        *p = nline;
        f++; p++;
    }
    char *c = (char *) malloc(6);
    *c = '$';
    shortToHexStr(c + 1, '\0');
    c[5] = '\0';
    struct line nline;
    nline.len = 6;
    nline.chars = c;
    *p = nline;
}

/**
 * Checks for pseudoops .blkw or .stringz
**/
int checkStringzBlkw(struct line *curline, int index) {
    char *f = curline->chars;
    char *stringz = STRINGZ;
    while (*stringz != '\0' && *f != '\0' && TOLOWER(*f) == *stringz) {
        f++; stringz++;
    }
    if (*stringz == '\0' && (*f == ' ' || *f == '\t')) {
        while (*f == ' ' || *f == '\t') f++;
        int size = 0;
        if ((size = parseStringz(f)) == -1) return -1;
        addLines(index, size - 1);
        addString(index, f);
        return size - 1;
    }
    f = curline->chars;
    char *blkw = BLKW;
    while (*blkw != '\0' && *f != '\0' && TOLOWER(*f) == *blkw) {
        f++; blkw++;
    }
    if (*blkw == '\0' && (*f == ' ' || *f == '\t')) {
        while (*f == ' ' || *f == '\t') f++;
        int size = 0;
        if ((size = parseBlkw(f)) == -1) return -1;
        addLines(index, size - 1);
        return size - 1;
    }
    return -2;
}

/**
 * check for .fill and parse with "$0000" 
 * 0000 = short hex
**/
int checkFill(struct line *curline) {
    char *f = curline->chars;
    char *fill = FILL;
    while (*fill != '\0' && *f != '\0' && TOLOWER(*f) == *fill) {
        fill++; f++;
    }
    if (*fill != '\0') return -2;
    if (*f != ' ' && *f != '\t') return -1;
    while (*f == ' ' || *f == '\t') f++;
    int val = 0;
    switch (*f) {
        case '0' :
            val = octalStrToInt(f);
            break;
        case 'x': case 'X':
            val = hexStrToInt(f);
            break;
        default:
            val = decStrToInt(f);
            break;
    }
    if (conversionerror) {
        printf(".fill not a valid number\n");
        return -1;
    }
    free(curline->chars);
    curline->chars = (char *) malloc(6);
    curline->chars[0] = '$';
    shortToHexStr(curline->chars + 1,val);
    curline->chars[5] = '\0';
    return 0;
}

/**
 * Create label table with locations in code
 * Parse .orig and .end blocks of code
 * Parse .blkw and .stringz pseudo-ops
**/
int firstPass(void) {
    char *first = lines.plines->chars;
    int orig = checkOrig(first); //-1 if missing, -2 if malformed
    if (orig == -2) {
        printf("Missing initial .orig statement\n");
        return -1;
    } else if (orig == -1) return -1;

    struct line *curline = lines.plines;
    curline++;
    int inc = 0;
    int end = lines.size - 1;
    while (inc < end - 1) {
        int checkend = 0;
        if ((checkend = checkEnd(curline)) == 0) {    //.end is there
            end += -1;
            curline = &lines.plines[lines.size - end + inc];
            orig = -1;
            continue;
        } else if (checkend == -1) return -1; //.end is malformed
        int chOrig = 0;

        if ((chOrig = checkOrig(curline->chars)) >= 0) {
            if (orig == -1) {
                end = end - inc;
                inc = 0;
                orig = chOrig;
                inc++; curline++;
                continue;
            } else {
                printf(".orig statement before .end statement\n");
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
        int memspace = 0;
        if ((memspace = checkStringzBlkw(curline, inc + lines.size - end)) >= 0) {
            inc += memspace;
            end += memspace;
            curline = &lines.plines[lines.size - end + inc];
        } else if (memspace == -1) return -1;

        int fill = 0;
        if ((fill = checkFill(curline)) == -1) return -1;

        inc++; curline++;
    }

    int checkFinalEnd = 0;
    if ((checkFinalEnd = checkEnd(curline)) == -2) {
        printf("Missing final .end statement\n" );
        return -1;
    } else if (checkFinalEnd == -1) return -1;

    return 0;
}
