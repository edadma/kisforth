#include "dictionary.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "debug.h"
#include "error.h"
#include "floating.h"
#include "forth.h"
#include "memory.h"
#include "stack.h"
#include "test.h"
#include "text.h"
#include "tools.h"

// Dictionary head points to the most recently defined word
word_t* dictionary_head = NULL;

// Initialize empty dictionary
void dictionary_init(void) {
  dictionary_head = NULL;
  create_primitives();
  create_builtin_definitions();

#ifdef FORTH_ENABLE_TOOLS
  create_tools_primitives();
  create_tools_definitions();
#endif

#ifdef FORTH_ENABLE_FLOATING
  create_floating_primitives();
  create_floating_definitions();
#endif

#ifdef FORTH_ENABLE_TESTS
  create_test_primitives();
#endif

#ifdef FORTH_DEBUG_ENABLED
  create_primitive_word("DEBUG-ON", f_debug_on);
  create_primitive_word("DEBUG-OFF", f_debug_off);
#endif
}

// Link a word into the dictionary (at the head of the linked list)
void link_word(word_t* word) {
  word->link = dictionary_head;  // Point to previous head
  dictionary_head = word;        // Make this word the new head
}

// Add this helper function to dictionary.c
static int case_insensitive_strcmp(const char* a, const char* b) {
  while (*a && *b) {
    int ca = tolower(*a);
    int cb = tolower(*b);
    if (ca != cb) return ca - cb;
    a++;
    b++;
  }
  return tolower(*a) - tolower(*b);
}

// Find a word in the dictionary by name (case-sensitive search)
word_t* find_word(context_t* ctx, const char* name) {
  word_t* word = search_word(name);

  if (!word) {
    error(ctx, "Word not found: %s", name);
  }

  return word;
}

word_t* search_word(const char* name) {
  word_t* current = dictionary_head;

  while (current != NULL) {
    if (case_insensitive_strcmp(current->name, name) == 0) {
      return current;
    }
    current = current->link;  // Move to next word in chain
  }

  return NULL;
}

// ============================================================================
// Word creation and execution utilities (moved from core.c)
// ============================================================================

// Create a primitive word in virtual memory
word_t* create_primitive_word(const char* name,
                              void (*cfunc)(context_t* ctx, word_t* self)) {
  // Allocate space for the word structure
  forth_addr_t word_addr = forth_allot(&main_context, sizeof(word_t));
  word_t* word = addr_to_ptr(NULL, word_addr);

  // Initialize the word structure
  word->link = NULL;  // Will be set by link_word()
  strncpy(word->name, name, sizeof(word->name) - 1);
  word->name[sizeof(word->name) - 1] = '\0';  // Ensure null termination
  word->flags = 0;
  word->cfunc = cfunc;
  word->param.address = here;
  word->param_type = PARAM_ADDRESS;
  link_word(word);

  return word;
}

// Create a variable word and return a C pointer to its value
cell_t* create_variable_word(const char* name, cell_t initial_value) {
  // Create the word header with f_address cfunc
  word_t* word = create_primitive_word(name, f_address);

  word->param.value = initial_value;  // Store value directly
  word->param_type = PARAM_VALUE;

  // Return C pointer to the parameter field for efficiency
  forth_addr_t word_addr = ptr_to_addr(&main_context, word);
  forth_addr_t param_value_addr = word_addr + offsetof(word_t, param.value);
  return (cell_t*)&forth_memory[param_value_addr];
}

// Create an area word (calls create_primitive_word with f_address)
void create_area_word(const char* name) {
  create_primitive_word(name, f_address);
}

// Create an immediate primitive word
word_t* create_immediate_primitive_word(const char* name,
                                        void (*cfunc)(context_t* ctx,
                                                      word_t* self)) {
  word_t* word = create_primitive_word(name, cfunc);
  word->flags |= WORD_FLAG_IMMEDIATE;
  return word;
}

// Compile a word reference into the current definition
void compile_word(context_t* ctx, word_t* word) {
  if (!word) {
    error(ctx, "Cannot compile NULL word");
  }

  // Get the word's execution token (address)
  forth_addr_t xt = ptr_to_addr(ctx, word);

  // Compile the execution token
  compile_cell(ctx, xt);

  debug("Compiled word '%s' at XT=%d", word->name ? word->name : "unnamed", xt);
}

// Compile a cell value into the current definition
void compile_cell(context_t* ctx, cell_t value) {
  // Store the value at HERE and advance HERE
  forth_store(ctx, here, value);
  here += sizeof(cell_t);

  debug("Compiled cell value %d at address %d", value, here - sizeof(cell_t));
}

// Helper function for creating new word definitions
word_t* defining_word(context_t* ctx,
                      void (*cfunc)(context_t* ctx, word_t* self)) {
  char name_buffer[32];

  // Parse the name for the new definition
  char* name = parse_name(ctx, name_buffer, sizeof(name_buffer));

  if (!name) error(ctx, "Missing name after ':'");

  debug("Creating word: %s", name);

  // Create word header but don't link it yet (hidden until ; is executed)
  forth_addr_t word_addr = forth_allot(ctx, sizeof(word_t));
  word_t* word = addr_to_ptr(NULL, word_addr);

  // Initialize word header
  strncpy(word->name, name, sizeof(word->name) - 1);
  word->name[sizeof(word->name) - 1] = '\0';
  word->flags = 0;
  word->cfunc = cfunc;
  word->param.address =
      here;  // Set parameter field to point to next free space
  word->param_type = PARAM_ADDRESS;
  link_word(word);

  return word;
}

// Execute a word by calling its cfunc
void execute_word(context_t* ctx, word_t* word) {
  require(ctx, word != NULL);
  require(ctx, word->cfunc != NULL);
  word->cfunc(ctx, word);
}

// Store string in Forth memory as counted string (ANS Forth style)
// Returns Forth address of the counted string
forth_addr_t store_counted_string(context_t* ctx, const char* str, int length) {
  // Align for the string
  forth_align();
  forth_addr_t string_addr = here;

  debug("store_counted_string: storing \"%s\" (length %d) at address %u", str,
        length, string_addr);

  // Allocate space for the entire counted string (length byte + characters)
  forth_allot(ctx, 1 + length);

  // Store length byte first (counted string format)
  forth_c_store(ctx, string_addr, (byte_t)length);

  // Store the string characters
  for (int i = 0; i < length; i++) {
    forth_c_store(ctx, string_addr + 1 + i, (byte_t)str[i]);
  }

  // Align after string storage for next allocation
  forth_align();

  debug("store_counted_string: stored at %u, HERE now %u", string_addr, here);
  return string_addr;
}

// Execute a colon definition using the return stack
void execute_colon(context_t* ctx, word_t* self) {
  // Parameter field contains array of tokens (word addresses)
  forth_addr_t tokens_addr =
      self->param.address;  // parameter field points to actual parameter
                            // space (word definition)

  debug("Executing colon definition: %s", self->name);

  // Save current instruction pointer on return stack (if executing)
  if (ctx->ip != 0) {
    return_push(ctx, (cell_t)ctx->ip);
    debug("  Saved IP on return stack");
  }

  // Set new instruction pointer to start of this definition's tokens
  ctx->ip = tokens_addr;

  // Execute tokens until EXIT is called (which will restore IP from return
  // stack)
  while (ctx->ip != 0) {
    forth_addr_t token_addr = forth_fetch(ctx, ctx->ip);
    ctx->ip += sizeof(cell_t);  // Advance to next token

    // Execute the word at token_addr
    word_t* word = addr_to_ptr(NULL, token_addr);
    debug("  Executing token: %s", word->name);
    execute_word(ctx, word);

    // If EXIT was called, ctx->ip will have been updated
  }

  debug("Colon definition execution complete");
}

// ============================================================================
// Word execution semantics
// ============================================================================

// VARIABLE runtime behavior: Push address OF the param.value
// (param contains the variable's value directly)
void f_address(context_t* ctx, word_t* self) {
  // Parameter field is right after the word structure in memory
  forth_addr_t word_addr = ptr_to_addr(ctx, self);
  forth_addr_t param_value_addr = word_addr + offsetof(word_t, param.value);

  // Push the Forth address of the parameter field
  data_push(ctx, param_value_addr);
}

// CREATE runtime behavior: Push address IN the param_field
// (param contains a Forth address pointing to data space)
void f_param_field(context_t* ctx, word_t* self) {
  data_push(ctx, self->param.address);  // Push the value stored in param_field
}