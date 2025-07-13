#include "forth.h"
#include "debug.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>


// Set the input buffer (like EVALUATE does)
/*
void set_input_buffer(const char* text) {
    if (!text) {
        input_length = 0;
        to_in = 0;
        input_buffer[0] = '\0';
        return;
    }

    size_t len = strlen(text);
    if (len >= INPUT_BUFFER_SIZE) {
        len = INPUT_BUFFER_SIZE - 1;
    }

    strncpy(input_buffer, text, len);
    input_buffer[len] = '\0';
    input_length = len;
    to_in = 0;

    debug("Input buffer set: \"%s\" (length=%d)\n", input_buffer, input_length);
}
*/

// Skip leading spaces in parse area (from >IN position)
void skip_spaces(void) {
    cell_t current_to_in = forth_fetch(to_in_addr);
    cell_t current_length = forth_fetch(input_length_addr);

    while (current_to_in < current_length &&
           isspace(forth_c_fetch(input_buffer_addr + current_to_in))) {
        current_to_in++;
           }

    forth_store(to_in_addr, current_to_in);
}

// Parse a name from the input buffer starting at >IN
// Returns pointer to parsed name (null-terminated) or NULL if end of input
// Updates >IN to point past the parsed name
char* parse_name(char* dest, size_t max_len) {
    skip_spaces();

    cell_t current_to_in = forth_fetch(to_in_addr);
    cell_t current_length = forth_fetch(input_length_addr);

    // Check if we're at end of input
    if (current_to_in >= current_length) {
        return NULL;
    }

    // Parse name until space or end of input
    size_t len = 0;
    while (current_to_in < current_length &&
           !isspace(forth_c_fetch(input_buffer_addr + current_to_in)) &&
           len < max_len - 1) {
        dest[len++] = forth_c_fetch(input_buffer_addr + current_to_in);
        current_to_in++;
           }

    dest[len] = '\0';

    // Update >IN in Forth memory
    forth_store(to_in_addr, current_to_in);

    return len > 0 ? dest : NULL;
}

// Try to parse token as a number (base 10)
// Returns true if successful, false if not a number
bool try_parse_number(const char* token, cell_t* result) {
    if (!token || !result) return false;

    char* endptr;
    long value = strtol(token, &endptr, 10);

    // Check if entire token was consumed (valid number)
    if (*endptr == '\0') {
        *result = (cell_t)value;
        return true;
    }

    return false;
}

// ANS Forth compliant text interpreter
// Implements the algorithm from section 3.4 of the standard
void interpret(void) {
    char name_buffer[64];

    // Get debug info from Forth memory
    cell_t current_length = forth_fetch(input_length_addr);
    char debug_buffer[INPUT_BUFFER_SIZE];
    for (cell_t i = 0; i < current_length && i < INPUT_BUFFER_SIZE - 1; i++) {
        debug_buffer[i] = forth_c_fetch(input_buffer_addr + i);
    }
    debug_buffer[current_length] = '\0';

    debug("Interpreting buffer: \"%s\"\n", debug_buffer);

    // Text interpretation loop (ANS Forth 3.4)
    while (forth_fetch(to_in_addr) < forth_fetch(input_length_addr)) {
        // a) Skip leading spaces and parse a name
        char* name = parse_name(name_buffer, sizeof(name_buffer));
        if (!name) {
            break;  // Parse area is empty
        }

        debug("  >IN=%d, parsed: '%s'", forth_fetch(to_in_addr), name);

        // b) Search the dictionary name space
        word_t* word = find_word(name);
        if (word) {
            debug(" -> found word, executing\n");
            // b.1) if interpreting, perform interpretation semantics
            execute_word(word);
        } else {
            // c) Not found, attempt to convert string to number
            cell_t number;
            if (try_parse_number(name, &number)) {
                debug(" -> number %d, pushing to stack\n", number);
                // c.1) if interpreting, place number on data stack
                data_push(number);
            } else {
                // d) If unsuccessful, ambiguous condition (error)
                printf(" -> ERROR: '%s' not found and not a number\n", name);
                return;  // Stop interpretation on error
            }
        }
    }

    debug("  Interpretation complete. >IN=%d, Stack depth: %d\n",
          forth_fetch(to_in_addr), data_depth());
    if (data_depth() > 0) {
        debug("  Top of stack: %d\n", data_peek());
    }
}

// Test accessor functions for the Forth memory input system
cell_t get_current_to_in(void) {
    return forth_fetch(to_in_addr);
}

cell_t get_current_input_length(void) {
    return forth_fetch(input_length_addr);
}

forth_addr_t get_current_input_buffer_addr(void) {
    return input_buffer_addr;
}
