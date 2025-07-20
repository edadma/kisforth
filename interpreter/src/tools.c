#include "tools.h"

#include <stdio.h>
#include <string.h>

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
static void f_dot_s(context_t* ctx, word_t* self) {
  (void)self;

  // Stack depth always in decimal for readability
  printf("<%d> ", data_depth(ctx));

  // Display each stack item in current BASE
  cell_t base = *base_ptr;

  // Validate base range, fall back to decimal if invalid
  if (base < 2 || base > 36) {
    base = 10;
  }

  for (int i = 0; i < ctx->data_stack_ptr; i++) {
    print_number_in_base(ctx->data_stack[i], base);
    printf(" ");
  }

  printf("\n");
  fflush(stdout);
}

// DUMP ( addr u -- ) Display u bytes starting at addr
static void f_dump(context_t* ctx, word_t* self) {
  (void)self;

  cell_t u = data_pop(ctx);
  forth_addr_t addr = (forth_addr_t)data_pop(ctx);

  if (u <= 0) return;

  printf("\nDUMP %08X (%d bytes):\n", addr, u);

  for (cell_t offset = 0; offset < u; offset += 16) {
    printf("%08X: ", addr + offset);

    // Hex bytes
    for (int i = 0; i < 16 && offset + i < u; i++) {
      if (i == 8) printf(" ");
      printf("%02X ", forth_c_fetch(ctx, addr + offset + i));
    }

    // Padding for short lines
    for (int i = u - offset; i < 16; i++) {
      if (i == 8) printf(" ");
      printf("   ");
    }

    printf(" |");

    // ASCII representation
    for (int i = 0; i < 16 && offset + i < u; i++) {
      byte_t ch = forth_c_fetch(ctx, addr + offset + i);
      putchar((ch >= 32 && ch <= 126) ? ch : '.');
    }

    printf("|\n");
  }
  printf("\n");
  fflush(stdout);
}

// WORDS ( -- ) Display the names of definitions in the first word list
static void f_words(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  int width = 16;
  int columns = 10;

  word_t* word = dictionary_head;
  int count = 0;

  printf("\nDictionary words:\n");
  while (word != NULL) {
    printf("%-*s ", width, word->name);
    count++;
    if (count % columns == 0) printf("\n");  // 6 words per line
    word = word->link;
  }
  if (count % columns != 0) printf("\n");
  printf("\n%d words\n", count);
  fflush(stdout);
}

// SEE ( "<spaces>name" -- ) Decompile word (simplified version)
static void f_see(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  char name_buffer[32];
  char* name = parse_name(ctx, name_buffer, sizeof(name_buffer));
  if (!name) {
    printf("SEE: Missing word name\n");
    return;
  }

  word_t* word = search_word(name);
  if (!word) {
    printf("SEE: Word '%s' not found\n", name);
    return;
  }

  printf("SEE %s\n", word->name);

  // Identify word type and display accordingly
  if (word->cfunc == execute_colon) {
    // Colon definition - decompile tokens
    printf(": %s ", word->name);

    forth_addr_t ip = word->param.address;
    while (true) {
      forth_addr_t token = forth_fetch(ctx, ip);
      ip += sizeof(cell_t);

      word_t* token_word = addr_to_ptr(ctx, token);

      // Check for EXIT (end of definition)
      if (strcmp(token_word->name, "EXIT") == 0) {
        break;
      }

      // Handle special cases
      if (strcmp(token_word->name, "LIT") == 0) {
        // Next token is a literal value
        cell_t literal = forth_fetch(ctx, ip);
        ip += sizeof(cell_t);
        printf("%d ", literal);
      } else if (strcmp(token_word->name, "0BRANCH") == 0) {
        forth_addr_t branch_addr = forth_fetch(ctx, ip);
        ip += sizeof(cell_t);
        printf("0BRANCH %u , ", branch_addr);
      } else if (strcmp(token_word->name, "BRANCH") == 0) {
        forth_addr_t branch_addr = forth_fetch(ctx, ip);
        ip += sizeof(cell_t);
        printf("BRANCH %u , ", branch_addr);
      } else if (strcmp(token_word->name, "(.\")") == 0) {
        cell_t length = forth_fetch(ctx, ip);
        ip += sizeof(cell_t);
        printf(".\" ");
        for (int i = 0; i < length; i++) {
          char ch = (char)forth_c_fetch(ctx, ip + i);
          putchar(ch);
        }
        printf("\" ");
        ip += length;
        ip = align_up(ip, sizeof(cell_t));
      } else if (strcmp(token_word->name, "(S\")") == 0) {
        // Next is string length, then string data
        cell_t length = forth_fetch(ctx, ip);
        ip += sizeof(cell_t);
        printf("S\" ");
        for (int i = 0; i < length; i++) {
          char ch = (char)forth_c_fetch(ctx, ip + i);
          putchar(ch);
        }
        printf("\" ");
        ip += length;
        ip = align_up(ip, sizeof(cell_t));  // Align after string
      } else {
        // Regular word
        printf("%s ", token_word->name);
      }
    }
  } else if (word->cfunc == f_address) {
    printf("VARIABLE %s  \\ current value: %d", word->name, word->param.value);
  } else if (word->cfunc == f_constant_runtime) {
    printf("%d CONSTANT %s", word->param.value, word->name);
  } else if (word->cfunc == f_value_runtime) {
    printf("%d VALUE %s", word->param.value, word->name);
  } else if (word->cfunc == f_param_field) {
    printf("CREATE %s  \\ data at address %u", word->name, word->param.address);
  } else {
    printf("<primitive> %s", word->name);
  }

  printf(" ;\n");
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
    interpret_text(&main_context, tools_definitions[i]);

    if (*state_ptr != 0)
      error(&main_context,
            "Tools definition left system in compilation state: %s",
            tools_definitions[i]);

    if (saved_state == 0 && *state_ptr != 0) {
      *state_ptr = saved_state;
    }
  }

  debug("Tools definitions complete");
}

#endif  // FORTH_ENABLE_TOOLS