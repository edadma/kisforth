#ifndef TEXT_H
#define TEXT_H

#include "types.h"
#include <stdbool.h>
#include <stddef.h>

// Input buffer management
void set_input_buffer(const char* text);
void set_current_to_in(cell_t value);

// Text parsing functions
void skip_spaces(void);
char* parse_name(char* dest, size_t max_len);
int parse_string(char quote_char, char* dest, size_t max_len);
bool try_parse_number(const char* token, cell_t* result);

// Text interpreter (ANS Forth compliant)
void interpret(void);
void interpret_text(const char* text);

// Compilation support
void compile_token(forth_addr_t token);
void compile_literal(cell_t value);

// Test accessor functions (for unit tests)
cell_t get_current_to_in(void);
cell_t get_current_input_length(void);
forth_addr_t get_current_input_buffer_addr(void);

#endif // TEXT_H