#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

// Stack and buffer sizes (defined here with the structure that uses them)
#define DATA_STACK_SIZE 256
#define RETURN_STACK_SIZE 256
#define PAD_SIZE 1024
#define WORD_BUFFER_SIZE 33
#define PICTURED_BUFFER_SIZE 66

#ifdef FORTH_ENABLE_FLOATING
#define FLOAT_STACK_SIZE 32
#endif

// Execution context structure
typedef struct context {
  // Execution state
  forth_addr_t ip;

  // Stacks (per-context)
  cell_t data_stack[DATA_STACK_SIZE];
  cell_t return_stack[RETURN_STACK_SIZE];
  int data_stack_ptr;
  int return_stack_ptr;

#ifdef FORTH_ENABLE_FLOATING
  // Floating point stack (per-context)
  double float_stack[FLOAT_STACK_SIZE];
  int float_stack_ptr;
#endif

  // Transient regions (per-context)
  byte_t pad_buffer[PAD_SIZE];
  byte_t word_buffer[WORD_BUFFER_SIZE];
  byte_t pictured_buffer[PICTURED_BUFFER_SIZE];

  // Input parsing state (per-context)
  const char* source_buffer;
  cell_t source_length;
  cell_t source_index;  // >IN equivalent

  // Context identification
  const char* name;  // "REPL", "TIMER_IRQ", etc.
  bool is_interrupt_handler;
} context_t;

// Context management functions
void context_init(context_t* ctx, const char* name, bool is_interrupt_handler);

#endif  // CONTEXT_H