
#include "version.h"

void printVersionInfo(FILE* file) {
    fprintf(file, PROGRAM_LONG " v" VERSION_STRING "\n");
    fprintf(file, "  version: " VERSION_STRING_BUILD "\n");
#ifdef GIT_URL
    fprintf(file, "  git: " GIT_URL "\n");
#endif
#ifdef LLVM
#ifdef LLVM_VERSION
    fprintf(file, "  LLVM backend: " LLVM_VERSION "\n");
#else
    fprintf(file, "  LLVM backend: yes\n");
#endif
#else
    fprintf(file, "  LLVM backend: no \n");
#endif
}

