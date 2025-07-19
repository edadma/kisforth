#ifndef STACK_H
#define STACK_H

#include "types.h"

// Data stack operations
void data_push(cell_t value);
cell_t data_pop(void);
cell_t data_peek(void);
cell_t data_peek_at(int offset);  // Peek at stack[depth-1-offset]
int data_depth(void);

// Return stack operations
void return_push(cell_t value);
cell_t return_pop(void);
int return_depth(void);
cell_t return_stack_peek(int offset);
cell_t* return_stack_data(void);
cell_t return_stack_peek_alternative(int offset);

#endif  // STACK_H