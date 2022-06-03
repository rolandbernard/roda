
#include <stdio.h>

#include "util/params.h"

#define COLUMN_WIDTH 25

void printOptionHelpLine(const char* single, const char* word, const char* value, const char* desc) {
    fputs("  ", stderr);
    int col = 2;
    if (single != NULL) {
        fputs("-", stderr);
        fputs(single, stderr);
        col += strlen(single) + 1;
    }
    if (word != NULL) {
        if (single != NULL) {
            fputs(" ", stderr);
            col += 1;
        }
        fputs("--", stderr);
        fputs(word, stderr);
        col += strlen(word) + 2;
    }
    if (value != NULL) {
        fputs("=", stderr);
        fputs(value, stderr);
        col += strlen(value) + 1;
    }
    if (col > COLUMN_WIDTH) {
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

