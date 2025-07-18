#include "text.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "memory.h"
#include "debug.h"
#include "floating.h"
#include "error.h"
#include "stack.h"
#include "dictionary.h"
#include "util.h"
#include "core.h"

// Set the input buffer (ANS Forth compliant version)
// This function sets up the input buffer in Forth memory space
void set_input_buffer(const char* text) {
    if (!text) {
        // Handle NULL input - set empty buffer
        forth_store(input_length_addr, 0);
        forth_store(to_in_addr, 0);
        forth_c_store(input_buffer_addr, '\0');
        return;
    }

    size_t len = strlen(text);

    // Bounds check - respect INPUT_BUFFER_SIZE limit
    if (len >= INPUT_BUFFER_SIZE) {
        len = INPUT_BUFFER_SIZE - 1;
    }

    // Copy string to Forth memory at input_buffer_addr
    for (size_t i = 0; i < len; i++) {
        forth_c_store(input_buffer_addr + i, text[i]);
    }

    // Null terminate
    forth_c_store(input_buffer_addr + len, '\0');

    // Set length and reset >IN to start of buffer
    forth_store(input_length_addr, (cell_t)len);
    forth_store(to_in_addr, 0);

    // Debug output (only if debug is enabled)
    debug("Input buffer set: \"%.*s\" (length=%d)\n",
          (int)len, text, (int)len);
}

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

// BASE-aware number parsing
// Returns true if successful, false if not a number
bool try_parse_number(const char* token, cell_t* result) {
    if (!token || !result) return false;

    cell_t base = *base_ptr;
    if (base < 2 || base > 36) {
        base = 10;  // Fall back to decimal for invalid base
    }

    const char* ptr = token;
    bool negative = false;
    cell_t value = 0;

    // Handle optional sign
    if (*ptr == '-') {
        negative = true;
        ptr++;
    } else if (*ptr == '+') {
        ptr++;
    }

    // Must have at least one digit after sign
    if (*ptr == '\0') return false;

    // Convert digits
    while (*ptr) {
        int digit = char_to_digit(*ptr, base);
        if (digit < 0) return false;  // Invalid digit

        // Check for overflow before multiplying
        if (value > (INT32_MAX - digit) / base) {
            return false;  // Would overflow
        }

        value = value * base + digit;
        ptr++;
    }

    *result = negative ? -value : value;
    return true;
}

// Helper function to set >IN (needed by parse_string)
void set_current_to_in(cell_t value) {
    forth_store(to_in_addr, value);
}

// Parse string delimited by quote character
// Returns length of parsed string, stores string at dest
// Updates >IN to position after closing quote
int parse_string(char quote_char, char* dest, size_t max_len) {
    skip_spaces();

    cell_t to_in = get_current_to_in();
    cell_t input_length = get_current_input_length();
    forth_addr_t input_addr = get_current_input_buffer_addr();

    int length = 0;
    bool found_closing_quote = false;

    debug("parse_string: looking for '%c' starting at >IN=%d, input_length=%d",
          quote_char, to_in, input_length);

    // Parse until we find the closing quote or end of input
    while (to_in < input_length && length < (int)(max_len - 1)) {
        byte_t ch = forth_c_fetch(input_addr + to_in);
        to_in++;

        if (ch == quote_char) {
            // Found closing quote - we're done
            found_closing_quote = true;
            debug("parse_string: found closing quote at position %d", to_in - 1);
            break;
        }

        dest[length++] = ch;
    }

    dest[length] = '\0';  // Null terminate

    // Update >IN to position after closing quote (or end of input)
    set_current_to_in(to_in);

    if (!found_closing_quote) {
        printf("WARNING: Missing closing %c in string\n", quote_char);
    }

    debug("parse_string: parsed \"%s\" (length %d)", dest, length);
    return length;
}

// Compile a token (word address) into current definition
void compile_token(forth_addr_t token) {
    if (*state_ptr == 0) error("Not compiling");

    // Align and store the token
    forth_align();
    forth_store(here, token);
    here += sizeof(cell_t);

    debug("Compiled token: %u at address %u", token, here - sizeof(cell_t));
}

// Compile a literal using LIT
void compile_literal(cell_t value) {
    // Find LIT word address
    word_t* lit_word = find_word("LIT");

    debug("Found LIT word at address %u", ptr_to_addr(lit_word));

    // Compile LIT followed by the literal value
    compile_token(ptr_to_addr(lit_word));
    compile_token((forth_addr_t)value);

    debug("Compiled literal: %d", value);
}

#ifdef FORTH_ENABLE_FLOATING

// Compile a float literal using FLIT
void compile_float_literal(double value) {
    if (*state_ptr == 0) error("Not compiling");

    // Find FLIT word address
    word_t* flit_word = find_word("FLIT");
    if (!flit_word) error("FLIT word not found");

    debug("Found FLIT word at address %u", ptr_to_addr(flit_word));

    // Compile FLIT followed by the 8-byte double value
    compile_token(ptr_to_addr(flit_word));

    // Store double as two consecutive 32-bit cells
    union {
        double d;
        uint32_t cells[2];
    } converter;
    converter.d = value;

    compile_token((forth_addr_t)converter.cells[0]);
    compile_token((forth_addr_t)converter.cells[1]);

    debug("Compiled float literal: %g", value);
}

#endif // FORTH_ENABLE_FLOATING

// ANS Forth compliant text interpreter
// Implements the algorithm from section 3.4 of the standard
// Enhanced to support both interpretation and compilation modes
void interpret(void) {
    char name_buffer[64];

    debug_with_buffer(debug_buffer, INPUT_BUFFER_SIZE, {
        cell_t current_length = forth_fetch(input_length_addr);
        for (cell_t i = 0; i < current_length && i < INPUT_BUFFER_SIZE - 1; i++) {
            debug_buffer[i] = forth_c_fetch(input_buffer_addr + i);
        }
        debug_buffer[current_length] = '\0';
    }, "Interpreting buffer: \"%s\" (STATE=%d)", debug_buffer, *state_ptr);

    // Text interpretation loop (ANS Forth 3.4)
    while (forth_fetch(to_in_addr) < forth_fetch(input_length_addr)) {
        // a) Skip leading spaces and parse a name
        char* name = parse_name(name_buffer, sizeof(name_buffer));
        if (!name) {
            break;  // Parse area is empty
        }

        debug("  >IN=%d, parsed: '%s'", forth_fetch(to_in_addr), name);

        // b) Search the dictionary name space
        word_t* word = search_word(name);

        if (word) {
            debug(" -> found word");

            // Check if word is immediate (always execute)
            if (word->flags & WORD_FLAG_IMMEDIATE) {
                debug(" (immediate), executing");
                execute_word(word);
            } else if (*state_ptr == 0) {
                // b.1) if interpreting, perform interpretation semantics
                debug(" (interpreting), executing");
                execute_word(word);
            } else {
                // b.2) if compiling, perform compilation semantics
                debug(" (compiling), compiling token");
                compile_token(ptr_to_addr(word));
            }
        } else {
            // c) Not found, attempt to convert string to number
            cell_t number;
            if (try_parse_number(name, &number)) {
                debug(" -> number %d", number);

                if (*state_ptr == 0) {
                    // c.1) if interpreting, place number on data stack
                    debug(" (interpreting), pushing to stack");
                    data_push(number);
                } else {
                    // c.2) if compiling, compile literal
                    debug(" (compiling), compiling literal");
                    compile_literal(number);
                }
            } else {
#ifdef FORTH_ENABLE_FLOATING
                // c3) Try parsing as floating-point number
                double float_number;
                if (try_parse_float(name, &float_number)) {
                    debug(" -> float %g", float_number);

                    if (*state_ptr == 0) {
                        // c3.1) if interpreting, place float on float stack
                        debug(" (interpreting), pushing to float stack");
                        float_push(float_number);
                    } else {
                        // c3.2) if compiling, compile float literal (not implemented yet)
                        debug(" (compiling), compiling float literal");
                        compile_float_literal(float_number);
                    }
                } else {
#endif
                    // d) If unsuccessful, ambiguous condition (error)
                    error("'%s' not found and not a number", name);
#ifdef FORTH_ENABLE_FLOATING
                }
#endif
            }
        }
    }

    debug("  Interpretation complete. >IN=%d, Stack depth: %d",
          forth_fetch(to_in_addr), data_depth());
    if (data_depth() > 0) {
        debug("  Top of stack: %d", data_peek());
    }
}

// Interpret text directly - convenience function
void interpret_text(const char* text) {
    set_input_buffer(text);
    interpret();
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
