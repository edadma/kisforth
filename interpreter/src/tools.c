#include "tools.h"

#include <stdio.h>

#include "core.h"
#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "stack.h"
#include "text.h"

#include "util.h"

#ifdef FORTH_ENABLE_TOOLS

// .S ( -- ) Display the contents of the data stack
void f_dot_s(word_t* self) {
  (void)self;

  // Stack depth always in decimal for readability
  printf("<%d> ", data_depth());

  // Display each stack item in current BASE
  cell_t base = *base_ptr;

  // Validate base range, fall back to decimal if invalid
  if (base < 2 || base > 36) {
    base = 10;
  }

  for (int i = 0; i < data_stack_ptr; i++) {
    print_number_in_base(data_stack[i], base);
    printf(" ");
  }

  printf("\n");
  fflush(stdout);
}

// DUMP ( addr u -- ) Display u bytes starting at addr
void f_dump(word_t* self) {
  (void)self;

  cell_t u = data_pop();
  forth_addr_t addr = (forth_addr_t)data_pop();

  if (u <= 0) return;

  printf("\nDUMP %08X (%d bytes):\n", addr, u);

  for (cell_t offset = 0; offset < u; offset += 16) {
    printf("%08X: ", addr + offset);

    // Hex bytes
    for (int i = 0; i < 16 && offset + i < u; i++) {
      if (i == 8) printf(" ");
      printf("%02X ", forth_c_fetch(addr + offset + i));
    }

    // Padding for short lines
    for (int i = u - offset; i < 16; i++) {
      if (i == 8) printf(" ");
      printf("   ");
    }

    printf(" |");

    // ASCII representation
    for (int i = 0; i < 16 && offset + i < u; i++) {
      byte_t ch = forth_c_fetch(addr + offset + i);
      putchar((ch >= 32 && ch <= 126) ? ch : '.');
    }

    printf("|\n");
  }
  printf("\n");
  fflush(stdout);
}

// WORDS ( -- ) Display the names of definitions in the first word list
void f_words(word_t* self) {
  (void)self;

  word_t* word = dictionary_head;
  int count = 0;

  printf("\nDictionary words:\n");
  while (word != NULL) {
    printf("%-12s ", word->name);
    count++;
    if (count % 6 == 0) printf("\n");  // 6 words per line
    word = word->link;
  }
  if (count % 6 != 0) printf("\n");
  printf("\n%d words\n", count);
  fflush(stdout);
}

// SEE ( "<spaces>name" -- ) Decompile word (simplified version)
void f_see(word_t* self) {
  (void)self;

  // This is a simplified version - full SEE requires parsing the next word
  printf("SEE: Decompiler not yet implemented\n");
  printf("Usage: Find word with FIND first, then examine with DUMP\n");
  fflush(stdout);
}

// Create all tools word set primitives
void create_tools_primitives(void) {
  create_primitive_word(".S", f_dot_s);
  create_primitive_word("DUMP", f_dump);
  create_primitive_word("WORDS", f_words);
  create_primitive_word("SEE", f_see);

  debug("Tools word set primitives created");
}

// Built-in Tools colon definitions
static const char* tools_definitions[] = {
    // Tools Extension words that can be defined in Forth
    ": ? @ . ;",  // ( addr -- ) Display contents of address
    //": DEPTH DATA-STACK-DEPTH ;",   // Will need DATA-STACK-DEPTH primitive

    NULL};

void create_tools_definitions(void) {
  debug("Creating Tools word set definitions...");

  for (int i = 0; tools_definitions[i] != NULL; i++) {
    debug("  Defining: %s", tools_definitions[i]);

    cell_t saved_state = *state_ptr;
    interpret_text(tools_definitions[i]);

    if (*state_ptr != 0)
      error(ctx, "Tools definition left system in compilation state: %s",
            tools_definitions[i]);

    if (saved_state == 0 && *state_ptr != 0) {
      *state_ptr = saved_state;
    }
  }

  debug("Tools definitions complete");
}

#endif  // FORTH_ENABLE_TOOLS