#ifndef CORE_H
#define CORE_H

#include "forth.h"

#define FORTH_PAD_SIZE 1024

// Global memory
extern cell_t* state_ptr;  // C pointer to STATE variable for efficiency
extern cell_t* base_ptr;   // BASE variable pointer

void create_primitives(void);
void create_builtin_definitions(void);

#endif  // CORE_H