#ifndef TOOLS_H
#define TOOLS_H

#ifdef FORTH_ENABLE_TOOLS

#include "forth.h"

// Tools word set functions
void f_dot_s(context_t* ctx, word_t* self);
void f_dump(context_t* ctx, word_t* self);
void f_words(context_t* ctx, word_t* self);
void f_see(context_t* ctx, word_t* self);

// Initialization functions
void create_tools_primitives(void);
void create_tools_definitions(void);

#endif  // FORTH_ENABLE_TOOLS

#endif  // TOOLS_H