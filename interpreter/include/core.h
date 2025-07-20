#ifndef CORE_H
#define CORE_H

#include "forth.h"

#define FORTH_PAD_SIZE 1024

// Global memory
extern cell_t* state_ptr;  // C pointer to STATE variable for efficiency
extern cell_t* base_ptr;   // BASE variable pointer

void f_constant_runtime(context_t* ctx, word_t* self);
void f_value_runtime(context_t* ctx, word_t* self);

void create_primitives(void);
void create_builtin_definitions(void);

#endif  // CORE_H