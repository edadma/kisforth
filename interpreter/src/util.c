#include "util.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "memory.h"

// Character array for digit conversion (supports bases 2-36)
static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Convert digit value (0-35) to character
char digit_to_char(int digit) {
    if (digit >= 0 && digit < 36) {
        return digits[digit];
    }
    return '?';  // Invalid digit
}

// Convert character to digit value in given base
int char_to_digit(char c, int base) {
    int digit = -1;

    if (c >= '0' && c <= '9') {
        digit = c - '0';
    } else if (c >= 'A' && c <= 'Z') {
        digit = c - 'A' + 10;
    } else if (c >= 'a' && c <= 'z') {
        digit = c - 'a' + 10;
    }

    return (digit >= 0 && digit < base) ? digit : -1;
}

// Print number in specified base (2-36)
void print_number_in_base(cell_t value, cell_t base) {
    char buffer[33];  // Enough for 32-bit binary + sign + null
    char* ptr = buffer + sizeof(buffer) - 1;
    *ptr = '\0';

    bool negative = false;
    uint32_t uvalue;

    // Handle sign for decimal output, treat as unsigned for other bases
    if (base == 10 && value < 0) {
        negative = true;
        uvalue = (uint32_t)(-value);
    } else {
        uvalue = (uint32_t)value;
    }

    // Convert digits (reverse order)
    do {
        --ptr;
        *ptr = digit_to_char(uvalue % base);
        uvalue /= base;
    } while (uvalue > 0);

    // Add sign if negative
    if (negative) {
        --ptr;
        *ptr = '-';
    }

    printf("%s", ptr);
}
