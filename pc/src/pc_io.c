#include "forth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For now, we'll use simple stdio. Later we can add readline support
// when we want to add -lreadline to the build

// PC I/O implementation
static void pc_print_string(const char* str) {
    printf("%s", str);
    fflush(stdout);
}

static void pc_print_char(char c) {
    putchar(c);
    fflush(stdout);
}

static char* pc_read_line(const char* prompt) {
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

static void pc_cleanup(void) {
    // Nothing special needed for basic stdio
}

// PC I/O interface
static io_interface_t pc_io = {
    .print_string = pc_print_string,
    .print_char = pc_print_char,
    .read_line = pc_read_line,
    .cleanup = pc_cleanup
};

// Get the PC I/O interface
io_interface_t* get_pc_io(void) {
    return &pc_io;
}