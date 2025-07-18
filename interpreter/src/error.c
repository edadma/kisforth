#include <stdio.h>
#include <stdarg.h>
#include "error.h"
#include "types.h"
#include "stack.h"
#include "repl.h"

void error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("ERROR: ");
    vprintf(format, args);
    va_end(args);

    putchar('\n');
    fflush(stdout);
    f_abort(NULL);
}

void f_abort(word_t* self) {
    // Empty data stack
    data_stack_ptr = 0;

    // Call QUIT for cleanup
    f_quit(self);
}