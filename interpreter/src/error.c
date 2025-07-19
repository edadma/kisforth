#include "error.h"

#include <stdarg.h>
#include <stdio.h>

#include "repl.h"
#include "stack.h"
#include "types.h"

void error(context_t* ctx, const char* format, ...) {
  va_list args;
  va_start(args, format);
  printf("ERROR: ");
  vprintf(format, args);
  va_end(args);

  putchar('\n');
  fflush(stdout);
  f_abort(ctx, NULL);
}

void f_abort(context_t* ctx, word_t* self) {
  // Empty data stack
  ctx->data_stack_ptr = 0;

  // Call QUIT for cleanup
  f_quit(ctx, self);
}