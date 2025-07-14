// In core/src/repl.c
#include "forth.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#define INPUT_BUFFER_SIZE 256

static char input_line[INPUT_BUFFER_SIZE];

// REPL control for QUIT word
static jmp_buf repl_restart;
static bool repl_running = false;

// QUIT word - restart the REPL loop
void f_quit(word_t* self) {
    (void)self;

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
    (void)self;

    printf("Goodbye!\n");
    exit(0);
}

// Character-by-character input with backspace support
static void get_line(void) {
    char *ptr = input_line;
    int c;

    while (ptr < input_line + INPUT_BUFFER_SIZE - 1) {
        c = getchar();

        if (c == '\r' || c == '\n') {
            *ptr = '\0';
            putchar('\n');
            fflush(stdout);
            break;
        } else if (c == '\b' || c == 127) { // Backspace or DEL
            if (ptr > input_line) {
                ptr--;
                printf("\b \b"); // Erase character visually
                fflush(stdout);
            }
        } else if (c >= 32 && c < 127) { // Printable characters
            *ptr++ = c;
            putchar(c);
            fflush(stdout);
        }
        // Ignore other characters (ctrl chars, etc.)
    }

    if (ptr >= input_line + INPUT_BUFFER_SIZE - 1) {
        *ptr = '\0';
    }
}

// Simplified REPL with setjmp for QUIT support
void forth_repl(void) {
    repl_running = true;

    printf("KISForth REPL - Type BYE to exit, QUIT to restart\n");

    // Set restart point for QUIT
    if (setjmp(repl_restart) != 0) {
        printf("Restarted.\n");
    }

    while (true) {
        printf(" ok\n");
        fflush(stdout);

        get_line();

        if (strlen(input_line) == 0) {
            continue;
        }

        set_input_buffer(input_line);
        interpret();

        if (data_depth() > 0) {
            printf(" <%d>", data_depth());
        }
    }

    repl_running = false;
}