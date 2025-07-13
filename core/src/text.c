#include "forth.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// ANS Forth compliant input buffer system
char input_buffer[INPUT_BUFFER_SIZE];
cell_t input_length = 0;
cell_t to_in = 0;  // >IN variable - current parse position

// Set the input buffer (like EVALUATE does)
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

    printf("Input buffer set: \"%s\" (length=%d)\n", input_buffer, input_length);
}

// SOURCE ( -- c-addr u ) Return input buffer address and length
char* source_addr(void) {
    return input_buffer;
}

cell_t source_length(void) {
    return input_length;
}

// Skip leading spaces in parse area (from >IN position)
void skip_spaces(void) {
    while (to_in < input_length && isspace(input_buffer[to_in])) {
        to_in++;
    }
}

// Parse a name from the input buffer starting at >IN
// Returns pointer to parsed name (null-terminated) or NULL if end of input
// Updates >IN to point past the parsed name
char* parse_name(char* dest, size_t max_len) {
    skip_spaces();

    // Check if we're at end of input
    if (to_in >= input_length) {
        return NULL;
    }

    // Parse name until space or end of input
    size_t len = 0;
    while (to_in < input_length &&
           !isspace(input_buffer[to_in]) &&
           len < max_len - 1) {
        dest[len++] = input_buffer[to_in++];
    }

    dest[len] = '\0';
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

    printf("Interpreting buffer: \"%s\"\n", input_buffer);

    // Text interpretation loop (ANS Forth 3.4)
    while (to_in < input_length) {
        // a) Skip leading spaces and parse a name
        char* name = parse_name(name_buffer, sizeof(name_buffer));
        if (!name) {
            break;  // Parse area is empty
        }

        printf("  >IN=%d, parsed: '%s'", to_in, name);

        // b) Search the dictionary name space
        word_t* word = find_word(name);
        if (word) {
            printf(" -> found word, executing\n");
            // b.1) if interpreting, perform interpretation semantics
            execute_word(word);
        } else {
            // c) Not found, attempt to convert string to number
            cell_t number;
            if (try_parse_number(name, &number)) {
                printf(" -> number %d, pushing to stack\n", number);
                // c.1) if interpreting, place number on data stack
                data_push(number);
            } else {
                // d) If unsuccessful, ambiguous condition (error)
                printf(" -> ERROR: '%s' not found and not a number\n", name);
                return;  // Stop interpretation on error
            }
        }
    }

    printf("  Interpretation complete. >IN=%d, Stack depth: %d\n", to_in, data_depth());
    if (data_depth() > 0) {
        printf("  Top of stack: %d\n", data_peek());
    }
}