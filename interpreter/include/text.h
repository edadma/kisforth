#ifndef TEXT_H
#define TEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "forth.h"

#define INPUT_BUFFER_SIZE 256

// Input buffer management
void set_input_buffer(context_t* ctx, const char* text);
void set_current_to_in(context_t* ctx, cell_t value);

// Text parsing functions
void skip_spaces(context_t* ctx);
char* parse_name(context_t* ctx, char* dest, size_t max_len);
int parse_string(context_t* ctx, char quote_char, char* dest, size_t max_len);
bool try_parse_number(const char* token, cell_t* result);

// Text interpreter (ANS Forth compliant)
void interpret(context_t* ctx);
void interpret_text(context_t* ctx, const char* text);

// Compilation support
void compile_token(context_t* ctx, forth_addr_t token);
void compile_literal(context_t* ctx, cell_t value);

// Test accessor functions (for unit tests)
cell_t get_current_to_in(context_t* ctx);
cell_t get_current_input_length(context_t* ctx);
forth_addr_t get_current_input_buffer_addr(void);

#endif  // TEXT_H