#ifndef UTIL_H
#define UTIL_H



// Character/digit conversion utilities
char digit_to_char(int digit);
int char_to_digit(char c, int base);

// Number formatting
void print_number_in_base(cell_t value, cell_t base);

#endif  // UTIL_H