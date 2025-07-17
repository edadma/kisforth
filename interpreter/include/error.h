#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>
#include "types.h"

// Error reporting function
void error(const char* format, ...);

// Forth ABORT word - clear data stack and restart
void f_abort(word_t* self);

// Requirement checking macro
#define require(condition, ...) \
do { \
if (!(condition)) { \
error("Requirement failed: %s at %s:%d - " __VA_ARGS__, \
#condition, __FILE__, __LINE__); \
} \
} while(0)

#endif // ERROR_H