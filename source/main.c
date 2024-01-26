#include "header.h"
#include "file_processing.h"

// #define _DIBUGGING_
#ifdef _DIBUGGING_

int main() {
    int argc = 3;
    char const *argv[] = {"neogit", "lkadj", "lidjf"};

#else

int main(int argc, char const *argv[]) {

#endif

    printf("Hello.\n");
    if (count_to_n(5));
    for (int i = argc - 1; i > 0; i--) printf("[%s] * ", argv[i]);
    return 0;
}