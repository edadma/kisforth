#include "forth.h"
#include <assert.h>
#include <string.h>

// Global virtual memory
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

// Allocate bytes in virtual memory and advance HERE
forth_addr_t forth_allot(size_t bytes) {
    assert(here + bytes <= FORTH_MEMORY_SIZE);  // Simple bounds check

    forth_addr_t old_here = here;
    here += bytes;

    // Zero the allocated memory
    memset(&forth_memory[old_here], 0, bytes);

    return old_here;
}

// Align HERE to cell boundary (4 bytes)
void forth_align(void) {
    while (here % sizeof(cell_t) != 0) {
        here++;
    }
}

// Convert Forth address to C pointer
word_t* addr_to_word(forth_addr_t addr) {
    assert(addr < FORTH_MEMORY_SIZE);
    return (word_t*)&forth_memory[addr];
}

// Convert C pointer to Forth address
forth_addr_t word_to_addr(word_t* word) {
    uint8_t* word_ptr = (uint8_t*)word;
    assert(word_ptr >= forth_memory);
    assert(word_ptr < forth_memory + FORTH_MEMORY_SIZE);

    return (forth_addr_t)(word_ptr - forth_memory);
}