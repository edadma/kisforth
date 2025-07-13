#include <stdio.h>
#include <stdlib.h>
#include "forth.h"

int main(int argc, char* argv[]) {
    printf("KISForth v0.0.1 - PC Development Version\n");
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);

    // Initialize the Forth system
    stack_init();
    dictionary_init();
    create_all_primitives();

    printf("Forth system initialized.\n");

    // Look up and execute the TEST word
    word_t* test_word = find_word("TEST");
    if (test_word) {
        printf("Running tests...\n\n");
        execute_word(test_word);
    } else {
        printf("TEST word not found. Tests may not be compiled in.\n");
        printf("Rebuild with -DENABLE_TESTS=ON to enable tests.\n");
        return 1;
    }

    return 0;
}