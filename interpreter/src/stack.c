#include "stack.h"

#include <stdio.h>

#include "error.h"
#include "forth.h"

// Data stack operations
void data_push(context_t* ctx, cell_t value) {
  require(ctx->data_stack_ptr < DATA_STACK_SIZE, "Stack overflow");
  ctx->data_stack[ctx->data_stack_ptr++] = value;
}

cell_t data_pop(context_t* ctx) {
  require(ctx->data_stack_ptr > 0, "Stack underflow");
  return ctx->data_stack[--ctx->data_stack_ptr];
}

cell_t data_peek(context_t* ctx) {
  require(ctx->data_stack_ptr > 0, "Stack underflow");
  return ctx->data_stack[ctx->data_stack_ptr - 1];
}

// Peek at specific stack position (0 = top, 1 = second from top, etc.)
cell_t data_peek_at(context_t* ctx, int offset) {
  require(ctx->data_stack_ptr > offset);  // Bounds check
  return ctx->data_stack[ctx->data_stack_ptr - 1 - offset];
}

int data_depth(context_t* ctx) { return ctx->data_stack_ptr; }

// Return stack operations
void return_push(context_t* ctx, cell_t value) {
  require(ctx->return_stack_ptr < RETURN_STACK_SIZE, "Return stack overflow");
  ctx->return_stack[ctx->return_stack_ptr++] = value;
}

cell_t return_pop(context_t* ctx) {
  require(ctx->return_stack_ptr > 0, "Return stack underflow");
  return ctx->return_stack[--ctx->return_stack_ptr];
}

int return_depth(context_t* ctx) { return ctx->return_stack_ptr; }

// Return stack peek function - examine return stack without popping
cell_t return_stack_peek(context_t* ctx, int offset) {
  if (return_depth() <= offset) {
    error(ctx, "Return stack underflow in peek at offset %d", offset);
  }
  // Access return stack from top (offset 0 = top, 1 = second from top, etc.)
  return ctx->return_stack[return_depth() - 1 - offset];
}
