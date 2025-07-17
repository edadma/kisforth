#include "forth.h"
#include "floating.h"
#include "debug.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#ifdef FORTH_ENABLE_FLOATING

// Float stack storage
double float_stack[FLOAT_STACK_SIZE];
int float_stack_ptr = 0;  // Points to next empty slot

// Initialize float stack
void float_stack_init(void) {
    float_stack_ptr = 0;
}

// Float stack operations
void float_push(double value) {
    require(float_stack_ptr < FLOAT_STACK_SIZE);  // Stack overflow check
    float_stack[float_stack_ptr++] = value;
    debug("Float pushed: %g (depth now %d)", value, float_stack_ptr);
}

double float_pop(void) {
    require(float_stack_ptr > 0);  // Stack underflow check
    double value = float_stack[--float_stack_ptr];
    debug("Float popped: %g (depth now %d)", value, float_stack_ptr);
    return value;
}

double float_peek(void) {
    require(float_stack_ptr > 0);  // Stack underflow check
    return float_stack[float_stack_ptr - 1];
}

int float_depth(void) {
    return float_stack_ptr;
}

// Liberal floating-point number parsing
// Accepts: 123.456, .5, 5., 1E5, 1.23e-10, etc.
// Rejects pure integers like: 123, -456 (those go to integer stack)
// Only works when BASE is decimal (ANS requirement)
bool try_parse_float(const char* token, double* result) {
    if (!token || !result) return false;

    // ANS Forth requirement: float parsing only in decimal base
    if (*base_ptr != 10) {
        return false;
    }

    // Empty string is not a float
    if (*token == '\0') return false;

    // Quick check: must contain decimal point OR exponent to be considered float
    // This distinguishes floats from integers: "123" -> integer, "123.0" -> float
    bool looks_like_float = (strchr(token, '.') != NULL ||
                            strchr(token, 'e') != NULL ||
                            strchr(token, 'E') != NULL);

    if (!looks_like_float) {
        return false;  // Pure integer, let try_parse_number handle it
    }

    // Use strtod to do the actual parsing
    char* endptr;
    double value = strtod(token, &endptr);

    // Check if entire string was consumed
    if (*endptr != '\0') {
        return false;  // Invalid characters or incomplete parse
    }

    // Check for valid result (reject NaN and infinity for safety)
    if (isnan(value) || isinf(value)) {
        return false;
    }

    *result = value;
    debug("Parsed float: '%s' -> %g", token, value);
    return true;
}

// FDROP ( F: r -- ) Remove top float from stack
void f_fdrop(struct word* self) {
    (void)self;
    float_pop();
}

// FDUP ( F: r -- r r ) Duplicate top float
void f_fdup(struct word* self) {
    (void)self;
    double r = float_peek();
    float_push(r);
}

// F+ ( F: r1 r2 -- r3 ) Add two floats
void f_f_plus(struct word* self) {
    (void)self;
    double r2 = float_pop();
    double r1 = float_pop();
    float_push(r1 + r2);
}

// F- ( F: r1 r2 -- r3 ) Subtract r2 from r1
void f_f_minus(struct word* self) {
    (void)self;
    double r2 = float_pop();
    double r1 = float_pop();
    float_push(r1 - r2);
}

// F* ( F: r1 r2 -- r3 ) Multiply two floats
void f_f_multiply(struct word* self) {
    (void)self;
    double r2 = float_pop();
    double r1 = float_pop();
    float_push(r1 * r2);
}

// F/ ( F: r1 r2 -- r3 ) Divide r1 by r2
void f_f_divide(struct word* self) {
    (void)self;
    double r2 = float_pop();
    double r1 = float_pop();

    // Check for division by zero
    if (r2 == 0.0) error("Floating-point division by zero in 'F/'");

    float_push(r1 / r2);
}

// F. ( F: r -- ) Display a float and remove from stack
void f_f_dot(struct word* self) {
    (void)self;
    double value = float_pop();

#ifdef FORTH_TARGET_PICO
    // Pico-specific formatting - manual cleanup for cleaner output
    char buffer[32];

    // Check if it's effectively a whole number
    if (fabs(value - round(value)) < 1e-9 && value >= -2147483648.0 && value <= 2147483647.0) {
        // Display as integer
        snprintf(buffer, sizeof(buffer), "%.0f", value);
    } else {
        // Use %.6g and manually clean up trailing zeros
        snprintf(buffer, sizeof(buffer), "%.6g", value);

        // Remove trailing zeros after decimal point (if any)
        char* dot = strchr(buffer, '.');
        if (dot && !strchr(buffer, 'e') && !strchr(buffer, 'E')) {
            char* end = buffer + strlen(buffer) - 1;
            while (end > dot && *end == '0') {
                *end = '\0';
                end--;
            }
            // Remove trailing decimal point if no digits after it
            if (*end == '.') {
                *end = '\0';
            }
        }
    }

    printf("%s ", buffer);
#else
    // PC build - use simple %g format (should work well)
    printf("%g ", value);
#endif

    fflush(stdout);
}

// FLIT implementation that reads from instruction stream
// FLIT ( F: -- r ) Push the float literal value that follows in compiled code
void f_flit(word_t* self) {
    (void)self;

    if (current_ip == 0) error("FLIT called outside colon definition");

    // Read the 8-byte double from instruction stream
    // Store it as two consecutive 32-bit cells, then convert back to double
    union {
        double d;
        uint32_t cells[2];
    } converter;

    converter.cells[0] = (uint32_t)forth_fetch(current_ip);
    current_ip += sizeof(cell_t);
    converter.cells[1] = (uint32_t)forth_fetch(current_ip);
    current_ip += sizeof(cell_t);

    // Push the float onto the float stack
    float_push(converter.d);

    debug("FLIT pushed float literal: %g", converter.d);
}

// Create all floating-point primitives
void create_floating_primitives(void) {
    create_primitive_word("FDROP", f_fdrop);
    create_primitive_word("FDUP", f_fdup);
    create_primitive_word("F+", f_f_plus);
    create_primitive_word("F-", f_f_minus);
    create_primitive_word("F*", f_f_multiply);
    create_primitive_word("F/", f_f_divide);
    create_primitive_word("F.", f_f_dot);
    create_primitive_word("FLIT", f_flit);

    debug("Floating-point primitives created");
}

// Built-in floating-point colon definitions (none for now)
static const char* floating_definitions[] = {
    // Future definitions like FDUP, FSWAP, etc. can be added here
    NULL  // End marker
};

// Create floating-point colon definitions
void create_floating_definitions(void) {
    debug("Creating floating-point definitions...");

    for (int i = 0; floating_definitions[i] != NULL; i++) {
        debug("  Defining: %s", floating_definitions[i]);

        cell_t saved_state = *state_ptr;
        interpret_text(floating_definitions[i]);

        if (*state_ptr != 0) {
            error("Floating-point definition left system in compilation state: %s", floating_definitions[i]);
        }

        if (saved_state == 0 && *state_ptr != 0) {
            *state_ptr = saved_state;
        }
    }

    debug("Floating-point definitions complete");
}

#endif // FORTH_ENABLE_FLOATING