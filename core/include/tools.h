#ifndef TOOLS_H
#define TOOLS_H

#include "forth.h"

#ifdef FORTH_ENABLE_TOOLS

// Tools word set functions
void f_dot_s(word_t* self);
void f_dump(word_t* self);
void f_words(word_t* self);
void f_see(word_t* self);

// Initialization functions
void create_tools_primitives(void);
void create_tools_definitions(void);

#endif // FORTH_ENABLE_TOOLS

#endif // TOOLS_H