#include <assert.h>
#include <stdio.h>
#include "stack.h"
#include "error.h"

// Stack storage
cell_t data_stack[DATA_STACK_SIZE];
cell_t return_stack[RETURN_STACK_SIZE];
int data_stack_ptr = 0;    // Points to next empty slot
int return_stack_ptr = 0;  // Points to next empty slot

// Initialize both stacks
void stack_init(void) {
    data_stack_ptr = 0;
    return_stack_ptr = 0;
}

// Data stack operations
void data_push(cell_t value) {
    require(data_stack_ptr < DATA_STACK_SIZE, "Stack overflow");
    data_stack[data_stack_ptr++] = value;
}

cell_t data_pop(void) {
    require(data_stack_ptr > 0, "Stack underflow");
    return data_stack[--data_stack_ptr];
}

cell_t data_peek(void) {
    require(data_stack_ptr > 0, "Stack underflow");
    return data_stack[data_stack_ptr - 1];
}

// Peek at specific stack position (0 = top, 1 = second from top, etc.)
cell_t data_peek_at(int offset) {
    require(data_stack_ptr > offset);  // Bounds check
    return data_stack[data_stack_ptr - 1 - offset];
}

int data_depth(void) {
    return data_stack_ptr;
}

// Return stack operations
void return_push(cell_t value) {
    require(return_stack_ptr < RETURN_STACK_SIZE, "Return stack overflow");
    return_stack[return_stack_ptr++] = value;
}

cell_t return_pop(void) {
    require(return_stack_ptr > 0, "Return stack underflow");
    return return_stack[--return_stack_ptr];
}

int return_depth(void) {
    return return_stack_ptr;
}