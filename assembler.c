#define  _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/***constants***/
#define ORIG ".orig"

/***structs***/
struct line {
    int len;
    char *chars;
};

struct totlines {
    int size;
    struct line *plines;
};

/***global data***/
struct totlines lines = {0, NULL};
FILE *fptr;
char conversionerror = 0; //will be set to anything but zero if conversion error

int openFile(char *filename) {
    fptr = fopen(filename, "r");
    if (!fptr) return 1;
    return 0;
}

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
        free(newline);
    }

    free(newline);
}

/***recursive pow function (may not use)***/
int pow(int a, int b) {
    if (b == 0) {
        return 1;
    } else if (b == 1) {
        return a;
    }
    if (b & 1) {
        return a * pow(a, b - 1);
    } else {
        int i = pow(a, b / 2);
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

/***
 * Checks for an initial .orig statement
 * @param f firstline of lines
 * @return 0 for success, -1 for error
***/
int checkOrig(char *f) {
    char *orig = ORIG;
    while (*orig != '\0') {
        if (*f != *orig) {
            printf("no .orig statement");
            return -1;
        }
        orig++; f++;
    }   //ensure space between .orig and number
    if (*f != ' ') {
        printf("malformed .orig statement"); 
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
        printf(".orig not a valid number");
        return -1;
    }
    return 0;
}

int firstPass() {
    char *firstLine = lines.plines->chars;

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Need file to assemble"); 
        return 1;
    }

    if (openFile(argv[1])) {
        printf("Error opening file"); 
        return 1;
    }

    printf("%d", decStrToInt("12296734"));

    parseFile();

    if (firstPass() == -1) return 1;


    return 0;
}