#ifndef REPL_H
#define REPL_H

#include "types.h"

#define INPUT_BUFFER_SIZE 256

// Main REPL system
void repl(void);

// REPL control primitives
void f_quit(word_t* self);       // QUIT ( -- ) Restart REPL loop
void f_bye(word_t* self);        // BYE ( -- ) Exit system

#endif // REPL_H