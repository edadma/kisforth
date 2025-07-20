#include "memory.h"

#include <assert.h>
#include <string.h>

#include "debug.h"
#include "error.h"
#include "forth.h"
#include "text.h"

/*
 * Virtual Memory
 * ==============
 * All Forth addresses are indices into forth_memory[].
 * This provides memory protection and 32-bit address consistency
 * across platforms, regardless of native pointer size.
 */
uint8_t forth_memory[FORTH_MEMORY_SIZE];
forth_addr_t here = 0;  // Data space pointer starts at beginning

forth_addr_t input_buffer_addr;
forth_addr_t to_in_addr;
forth_addr_t input_length_addr;

void input_system_init(void) {
  input_buffer_addr = forth_allot(INPUT_BUFFER_SIZE);
  to_in_addr = forth_allot(sizeof(cell_t));
  input_length_addr = forth_allot(sizeof(cell_t));

  // Initialize >IN to 0
  forth_store(to_in_addr, 0);
  forth_store(input_length_addr, 0);
}

// Store cell (32-bit) at Forth address - implements ! (STORE)
void forth_store(context_t* ctx, forth_addr_t addr, cell_t value) {
  require(ctx, addr + sizeof(cell_t) <= FORTH_MEMORY_SIZE);
  *(cell_t*)&forth_memory[addr] = value;
}

// Fetch cell (32-bit) from Forth address - implements @ (FETCH)
cell_t forth_fetch(context_t* ctx, forth_addr_t addr) {
  require(ctx, addr + sizeof(cell_t) <= FORTH_MEMORY_SIZE);
  return *(cell_t*)&forth_memory[addr];
}

// Store byte at Forth address - implements C! (C-STORE)
void forth_c_store(context_t* ctx, forth_addr_t addr, byte_t value) {
  require(ctx, addr < FORTH_MEMORY_SIZE);
  forth_memory[addr] = value;
}

// Fetch byte from Forth address - implements C@ (C-FETCH)
byte_t forth_c_fetch(context_t* ctx, forth_addr_t addr) {
  require(ctx, addr < FORTH_MEMORY_SIZE);
  return forth_memory[addr];
}

// Allocate bytes in virtual memory and advance HERE
forth_addr_t forth_allot(context_t* ctx, size_t bytes) {
  // Ensure allocation starts on aligned boundary
  forth_align();

  require(ctx, here + bytes <= FORTH_MEMORY_SIZE);  // Simple bounds check

  forth_addr_t old_here = here;
  here += bytes;

  // Zero the allocated memory
  memset(&forth_memory[old_here], 0, bytes);

  // Ensure HERE remains aligned for future allocations
  // This maintains the invariant that HERE always points to an
  // aligned address, preventing alignment issues in subsequent
  // cell allocations regardless of the size of this allocation
  forth_align();

  return old_here;
}

inline uintptr_t align_up(uintptr_t addr, size_t alignment) {
  return addr + alignment - 1 & ~(alignment - 1);
}

// Align HERE to cell boundary (4 bytes)
void forth_align(void) { here = align_up(here, sizeof(cell_t)); }

// Convert C pointer to Forth address
forth_addr_t ptr_to_addr(context_t* ctx, word_t* word) {
  uint8_t* word_ptr = (uint8_t*)word;
  require(ctx, word_ptr >= forth_memory);
  require(ctx, word_ptr < forth_memory + FORTH_MEMORY_SIZE);

  return word_ptr - forth_memory;
}