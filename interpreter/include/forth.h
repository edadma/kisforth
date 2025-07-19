#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include <stdint.h>

// Core Forth data types
typedef int32_t cell_t;         // 32-bit cell
typedef uint32_t ucell_t;       // Unsigned cell
typedef uint8_t byte_t;         // Byte for C@ C! operations
typedef uint32_t forth_addr_t;  // Forth address (always 32-bit)

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
typedef struct forth {
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

// Word structure - core to the entire Forth system
typedef struct word {
  struct word* link;  // Link to previous word (C pointer)
  char name[32];      // Word name (31 chars max per standard)
  uint32_t flags;     // Immediate flag, etc.
  void (*cfunc)(context_t* ctx,
                struct word* self);  // C function for ALL word types
  // DUAL-PURPOSE PARAMETER FIELD:
  // - For most words: Forth address pointing to parameter space
  // - For variables:  Direct storage of the variable's value
  uint32_t param_field;  // either value or Forth address
} word_t;

// Word flags
#define WORD_FLAG_IMMEDIATE 0x01

// Forth virtual memory size (could be redefined elsewhere)
#ifndef FORTH_MEMORY_SIZE
#define FORTH_MEMORY_SIZE (64 * 1024)  // 64KB virtual memory (default)
#endif

extern context_t main_context;

// Context management functions
void context_init(context_t* ctx, const char* name, bool is_interrupt_handler);
void* addr_to_ptr(context_t* ctx, forth_addr_t addr);

#endif  // CONTEXT_H