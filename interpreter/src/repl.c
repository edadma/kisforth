#include "repl.h"
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdarg.h>
#include "types.h"
#include "core.h"
#include "stack.h"
#include "text.h"
#include "line_editor.h"

static char input_line[INPUT_BUFFER_SIZE];

// REPL control for QUIT word
static jmp_buf repl_restart;
static bool repl_running = false;

// QUIT word - restart the REPL loop
void f_quit(word_t* self) {
    (void)self;

    if (repl_running) {
		current_ip = 0;
        return_stack_ptr = 0;
        *state_ptr = 0;

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
/*
static void get_line(void) {
    char *ptr = input_line;
    int c;

    while (ptr < input_line + INPUT_BUFFER_SIZE - 1) {
        c = getchar();

        if (c == '\r' || c == '\n') {
#ifdef FORTH_TARGET_PICO
           	putchar('\n');  // Echo newline on Pico
			fflush(stdout);
#endif
            *ptr = '\0';
            break;
        } else if (c == '\b' || c == 127) { // Backspace or DEL
            if (ptr > input_line) {
                ptr--;
                printf("\b \b"); // Erase character visually
                fflush(stdout);
            }
        } else if (c >= 32 && c < 127) { // Printable characters
            *ptr++ = c;
#ifdef FORTH_TARGET_PICO
            putchar(c);      // Echo character on Pico only
            fflush(stdout);
#endif
        }
        // Ignore other characters (ctrl chars, etc.)
    }

    if (ptr >= input_line + INPUT_BUFFER_SIZE - 1) {
        *ptr = '\0';
    }
}
*/

static void get_line(void) {
    enhanced_get_line(input_line, INPUT_BUFFER_SIZE);
}

// Simplified REPL with setjmp for QUIT support
void repl(void) {
    repl_running = true;

    // Set restart point for QUIT
    if (setjmp(repl_restart) != 0) {
        printf("Restarted.\n");
    }

    while (true) {
        printf(*state_ptr ? "\ncompiling> " : "\nok> ");
        fflush(stdout);

        get_line();

        if (strlen(input_line) == 0) {
            continue;
        }

        interpret_text(input_line);

        if (data_depth() > 0) {
            printf(" <%d>", data_depth());
        }
    }

    repl_running = false;
}