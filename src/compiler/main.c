
#include <stdlib.h>
#include <locale.h>

int main(int argc, const char* const* argv) {
    setlocale(LC_ALL, ""); // Set locale to user preference
    return EXIT_SUCCESS;
}

