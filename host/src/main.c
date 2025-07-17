#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "forth.h"
#include "memory.h"
#include "debug.h"
#include "version.h"

int main(int argc, char* argv[]) {
    printf("KISForth v%s - Host Development Version\n", KISFORTH_VERSION_STRING);
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);
    //printf("word_t size = %lu\n", sizeof(word_t));

    // Initialize the Forth system
    stack_init();

	#ifdef FORTH_DEBUG_ENABLED
   	debug_init();
	#endif

    input_system_init();    // Initialize input buffers in Forth memory
    dictionary_init();

    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        // Run tests and exit
        word_t* test_word = find_word("TEST");

        printf("Running tests...\n\n");
        execute_word(test_word);
    } else {
        // Start interactive REPL
        forth_repl();
    }

    return 0;
}