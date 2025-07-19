#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include "context.h"

// Core Forth data types
typedef int32_t cell_t;         // 32-bit cell
typedef uint32_t ucell_t;       // Unsigned cell
typedef uint8_t byte_t;         // Byte for C@ C! operations
typedef uint32_t forth_addr_t;  // Forth address (always 32-bit)

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

// Memory layout constants
#ifndef FORTH_MEMORY_SIZE
#define FORTH_MEMORY_SIZE (64 * 1024)  // 64KB virtual memory (default)
#endif

#endif  // TYPES_H