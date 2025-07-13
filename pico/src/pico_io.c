#include "forth.h"
#include <stdio.h>
#include <string.h>

// Pico I/O implementation using stdio over USB
static void pico_print_string(const char* str) {
    printf("%s", str);
    fflush(stdout);
}

static void pico_print_char(char c) {
    putchar(c);
    fflush(stdout);
}

static char* pico_read_line(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);

    static char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Remove newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        return buffer;
    }
    return NULL;
}

static void pico_cleanup(void) {
    // Nothing special needed
}

// Pico I/O interface
static io_interface_t pico_io = {
    .print_string = pico_print_string,
    .print_char = pico_print_char,
    .read_line = pico_read_line,
    .cleanup = pico_cleanup
};

// Get the Pico I/O interface
io_interface_t* get_pico_io(void) {
    return &pico_io;
}