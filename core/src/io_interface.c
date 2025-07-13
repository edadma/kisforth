#include "forth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

// Global I/O interface pointer
io_interface_t* current_io = NULL;

// Global REPL control
static jmp_buf repl_restart;
static bool repl_running = false;

// Set the I/O interface
void set_io_interface(io_interface_t* io) {
    current_io = io;
}

// Platform-agnostic I/O functions
void io_print(const char* str) {
    if (current_io && current_io->print_string) {
        current_io->print_string(str);
    } else {
        printf("%s", str);  // Fallback
    }
}

void io_print_char(char c) {
    if (current_io && current_io->print_char) {
        current_io->print_char(c);
    } else {
        putchar(c);  // Fallback
    }
}

char* io_read_line(const char* prompt) {
    if (current_io && current_io->read_line) {
        return current_io->read_line(prompt);
    } else {
        // Simple fallback
        io_print(prompt);
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
}

void io_cleanup(void) {
    if (current_io && current_io->cleanup) {
        current_io->cleanup();
    }
}

// QUIT word - restart the REPL loop
void f_quit(word_t* self) {
    if (repl_running) {
        // Clear stacks and reset input
        stack_init();
        set_input_buffer("");

        // Jump back to REPL start
        longjmp(repl_restart, 1);
    }
}

// BYE word - exit the system
void f_bye(word_t* self) {
    io_print("Goodbye!\n");
    io_cleanup();
    exit(0);
}

// Main REPL loop
void forth_repl(void) {
    repl_running = true;

    io_print("KISForth REPL - Type BYE to exit, QUIT to restart\n");

    // Set restart point for QUIT
    if (setjmp(repl_restart) != 0) {
        io_print("Restarted.\n");
    }

    while (true) {
        // Read a line of input
        char* line = io_read_line(" ok\n");
        if (!line) {
            break;  // EOF or error
        }

        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }

        // Set up input buffer and interpret
        set_input_buffer(line);
        interpret();

        // Show stack if not empty (debug feature)
        if (data_depth() > 0) {
            char stack_msg[64];
            snprintf(stack_msg, sizeof(stack_msg), " <%d>", data_depth());
            io_print(stack_msg);
        }
    }

    repl_running = false;
}