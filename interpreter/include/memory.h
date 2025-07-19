#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "types.h"

// Global memory
extern uint8_t forth_memory[FORTH_MEMORY_SIZE];
extern forth_addr_t here;  // Data space pointer

// Input system globals
extern forth_addr_t input_buffer_addr;
extern forth_addr_t to_in_addr;
extern forth_addr_t input_length_addr;

// Memory management functions
forth_addr_t forth_allot(size_t bytes);
void forth_align(void);
forth_addr_t ptr_to_addr(struct word* word);
uintptr_t align_up(uintptr_t addr, size_t alignment);

// Memory access functions
void forth_store(forth_addr_t addr, cell_t value);    // !
cell_t forth_fetch(forth_addr_t addr);                // @
void forth_c_store(forth_addr_t addr, byte_t value);  // C!
byte_t forth_c_fetch(forth_addr_t addr);              // C@

// Input system functions
void input_system_init(void);

#endif  // MEMORY_H