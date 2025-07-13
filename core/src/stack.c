#include "forth.h"
#include <assert.h>
#include <stdio.h>

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
    assert(data_stack_ptr < DATA_STACK_SIZE);  // Stack overflow check
    data_stack[data_stack_ptr++] = value;
}

cell_t data_pop(void) {
    assert(data_stack_ptr > 0);  // Stack underflow check
    return data_stack[--data_stack_ptr];
}

cell_t data_peek(void) {
    assert(data_stack_ptr > 0);  // Stack underflow check
    return data_stack[data_stack_ptr - 1];
}

// Peek at specific stack position (0 = top, 1 = second from top, etc.)
cell_t data_peek_at(int offset) {
    assert(data_stack_ptr > offset);  // Bounds check
    return data_stack[data_stack_ptr - 1 - offset];
}

int data_depth(void) {
    return data_stack_ptr;
}

// Return stack operations
void return_push(cell_t value) {
    assert(return_stack_ptr < RETURN_STACK_SIZE);  // Stack overflow check
    return_stack[return_stack_ptr++] = value;
}

cell_t return_pop(void) {
    assert(return_stack_ptr > 0);  // Stack underflow check
    return return_stack[--return_stack_ptr];
}

int return_depth(void) {
    return return_stack_ptr;
}