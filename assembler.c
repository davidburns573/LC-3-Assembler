#define  _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

struct line {
    int len;
    char *chars;
};

struct totlines {
    int size;
    struct line *plines;
};

struct totlines lines;
FILE *fptr;

int openFile(char *filename) {
    fptr = fopen(filename, "r");
    if (!fptr) return 1;
    return 0;
}

void addLine(int len, char *newline) {
    printf("%d\n%s", len, newline);
    lines.size = lines.size + 1;
    lines.plines = (struct line *) 
                realloc(lines.plines, lines.size * sizeof(struct line));

    struct line nline;
    nline.len = len;
    nline.chars = malloc(len);
    memcpy(nline.chars, newline, len);

    lines.plines[lines.size - 1] = nline;
}

int firstPass(void) {
    lines.size = 0;
    lines.plines = NULL;
    size_t linecap = 0;
    char *newline = NULL;
    int linelen;
    while ((linelen = getline(&newline, &linecap, fptr)) != -1) {
        if (linelen != 0) {                //remove empty lines
            int i = 0;
            while (newline[i] == ' ') i++; //remove white space
            if (newline[i] != ';')         //remove comments
                addLine(linelen - i, (newline + i));
        }
    }

    free(newline);
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

    firstPass();

    int i = 0;
    while (i < lines.size) {
        printf("%s", (lines.plines + i)->chars);
        i++;
    }

    return 0;
}