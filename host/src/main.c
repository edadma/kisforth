#include <stdio.h>
#include <string.h>
#include "repl.h"
#include "dictionary.h"
#include "startup.h"

int main(int argc, char* argv[]) {
    forth_system_init();
    print_startup_banner("Host Development");

    if (argc > 1 && strcmp(argv[1], "test") == 0) {
        word_t* test_word = find_word("TEST");
        printf("Running tests...\n\n");
        execute_word(test_word);
    } else {
        repl();
    }

    return 0;
}