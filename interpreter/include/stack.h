#ifndef STACK_H
#define STACK_H

#include "forth.h"

// Data stack operations
void data_push(context_t* ctx, cell_t value);
cell_t data_pop(context_t* ctx);
cell_t data_peek(context_t* ctx);
cell_t data_peek_at(context_t* ctx,
                    int offset);  // Peek at stack[depth-1-offset]
int data_depth(context_t* ctx);

// Return stack operations
void return_push(context_t* ctx, cell_t value);
cell_t return_pop(context_t* ctx);
int return_depth(context_t* ctx);
cell_t return_stack_peek(context_t* ctx, int offset);

#endif  // STACK_H