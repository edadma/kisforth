#ifndef ERROR_H
#define ERROR_H

#include "forth.h"

// Error reporting function
void error(context_t* ctx, const char* format, ...);

// Forth ABORT word - clear data stack and restart
void f_abort(context_t* ctx, word_t* self);

// Requirement checking macro
#define require(ctx, condition, ...)                                           \
  do {                                                                         \
    if (!(condition)) {                                                        \
      error(ctx, "Requirement failed: %s at %s:%d - " __VA_ARGS__, #condition, \
            __FILE__, __LINE__);                                               \
    }                                                                          \
  } while (0)

#endif  // ERROR_H