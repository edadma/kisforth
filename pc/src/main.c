#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"
#include "debug.h"
#include "version.h"

int main(int argc, char* argv[]) {
    printf("KISForth v%s - PC Development Version\n", KISFORTH_VERSION_STRING);
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);

    // Initialize the Forth system
    stack_init();
    debug_init();
    input_system_init();    // Initialize input buffers in Forth memory
    dictionary_init();
    create_all_primitives();

    printf("Forth system initialized.\n");

    // Check for command line arguments
    bool run_tests = false;
    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        run_tests = true;
    }

    if (run_tests) {
        // Run tests and exit
        word_t* test_word = find_word("TEST");
        if (test_word) {
            printf("Running tests...\n\n");
            execute_word(test_word);
        } else {
            printf("TEST word not found. Tests may not be compiled in.\n");
            printf("Rebuild with -DENABLE_TESTS=ON to enable tests.\n");
            return 1;
        }
    } else {
        // Start interactive REPL
        forth_repl();
    }

    return 0;
}