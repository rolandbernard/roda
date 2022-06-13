
#include <stdio.h>

#include "util/params.h"

#define COLUMN_WIDTH 27

void printOptionHelpLine(char single, const char* word, const char* value, const char* desc) {
    fputs("  ", stderr);
    int col = 2;
    if (single != 0) {
        fputs("-", stderr);
        fputc(single, stderr);
        col += 2;
    }
    if (word != NULL) {
        if (single != 0) {
            fputs(" ", stderr);
            col += 1;
        }
        fputs("--", stderr);
        fputs(word, stderr);
        col += strlen(word) + 2;
    }
    if (value != NULL) {
        fputs(value, stderr);
        col += strlen(value);
    }
    if (col >= COLUMN_WIDTH) {
        fputs("\n", stderr);
        col = 0;
    }
    while (col < COLUMN_WIDTH) {
        fputs(" ", stderr);
        col++;
    }
    fputs(desc, stderr);
    fputs("\n", stderr);
}

