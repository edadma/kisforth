#include "stack.h"

#include <assert.h>
#include <stdio.h>

#include "error.h"

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

int data_depth(void) { return data_stack_ptr; }

// Return stack operations
void return_push(cell_t value) {
  require(return_stack_ptr < RETURN_STACK_SIZE, "Return stack overflow");
  return_stack[return_stack_ptr++] = value;
}

cell_t return_pop(void) {
  require(return_stack_ptr > 0, "Return stack underflow");
  return return_stack[--return_stack_ptr];
}

int return_depth(void) { return return_stack_ptr; }

// Return stack peek function - examine return stack without popping
cell_t return_stack_peek(int offset) {
  if (return_depth() <= offset) {
    error(ctx, "Return stack underflow in peek at offset %d", offset);
  }
  // Access return stack from top (offset 0 = top, 1 = second from top, etc.)
  return return_stack_data()[return_depth() - 1 - offset];
}

// Helper function to get direct access to return stack data
// This may need to be implemented in stack.c depending on the current
// architecture
cell_t* return_stack_data(void) {
  // This function needs to return a pointer to the return stack array
  // Implementation depends on how the return stack is currently implemented
  extern cell_t return_stack[];  // Assuming this exists in stack.c
  return return_stack;
}

// Alternative simpler implementation using existing return stack functions
// Use this if direct access to return_stack array is not available
cell_t return_stack_peek_alternative(int offset) {
  if (return_depth() <= offset) {
    error(ctx, "Return stack underflow in peek at offset %d", offset);
  }

  // Save current return stack contents
  cell_t temp[64];  // Assuming max return stack depth
  int depth = return_depth();

  // Pop everything to temporary storage
  for (int i = 0; i < depth; i++) {
    temp[i] = return_pop();
  }

  // Get the value at the desired offset
  cell_t result = temp[offset];

  // Restore the return stack
  for (int i = depth - 1; i >= 0; i--) {
    return_push(temp[i]);
  }

  return result;
}
