#include "repl.h"

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core.h"
#include "forth.h"
#include "line_editor.h"
#include "stack.h"
#include "text.h"

static char input_line[INPUT_BUFFER_SIZE];

// REPL control for QUIT word
static jmp_buf repl_restart;
static bool repl_running = false;
static context_t repl_context;

// QUIT word - restart the REPL loop
void f_quit(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (repl_running) {
    ctx->ip = 0;
    ctx->return_stack_ptr = 0;
    *state_ptr = 0;

    // Jump back to REPL start
    longjmp(repl_restart, 1);
  }
}

// BYE word - exit the system
void f_bye(context_t* ctx, word_t* self) {
  (void)ctx;
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

static void get_line(void) { enhanced_get_line(input_line, INPUT_BUFFER_SIZE); }

// Simplified REPL with setjmp for QUIT support
void repl(void) {
  context_init(&repl_context, "REPL", false);
  repl_running = true;

  // Set restart point for QUIT
  if (setjmp(repl_restart) != 0) {
    printf("Restarted.\n");
  }

  for (;;) {
    printf(*state_ptr ? "\ncompiling> " : "\nok> ");
    fflush(stdout);

    get_line();

    if (strlen(input_line) == 0) {
      continue;
    }

    interpret_text(&main_context, input_line);

    if (data_depth(&repl_context) > 0) {
      printf(" <%d>", data_depth(&repl_context));
    }
  }
}