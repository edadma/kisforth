#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "repl.h"
#include "startup.h"

int main(int argc, char* argv[]) {
  forth_system_init();

  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    word_t* test_word = find_word(NULL, "TEST");
    printf("Running tests...\n\n");
    execute_word(ctx, test_word);
  } else {
    print_startup_banner("Host Development");
    repl();
  }

  return 0;
}