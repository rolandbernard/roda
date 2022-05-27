
#include <unistd.h>

#include "util/console.h"

bool isATerminal(FILE* stream) {
    return isatty(fileno(stream));
}
