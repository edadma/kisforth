#ifndef REPL_H
#define REPL_H

#include "context.h"
#include "types.h"

// Main REPL system
void repl(void);

// REPL control primitives
void f_quit(context_t* ctx, word_t* self);  // QUIT ( -- ) Restart REPL loop
void f_bye(context_t* ctx, word_t* self);   // BYE ( -- ) Exit system

#endif  // REPL_H