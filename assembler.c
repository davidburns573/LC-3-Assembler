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

struct totlines lines = {0, NULL};
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

int parseFile(void) {
    size_t linecap = 0;
    char *newline = NULL;
    int linelen;
    while ((linelen = getline(&newline, &linecap, fptr)) != -1) {
        int i = 0;
        while (newline[i] == ' ') i++;  //remove white space
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

    parseFile();

    int i = 0;
    while (i < lines.size) {
        printf("\n%s\n%d", (lines.plines + i)->chars, (lines.plines + i)->len);
        i++;
    }

    return 0;
}