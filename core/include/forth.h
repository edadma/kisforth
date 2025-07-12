#ifndef FORTH_H
#define FORTH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Core data types from design document
typedef int32_t cell_t;        // 32-bit cell
typedef uint32_t ucell_t;      // Unsigned cell
typedef uint8_t byte_t;        // Byte for C@ C! operations
typedef uint32_t forth_addr_t; // Forth address (always 32-bit)

// Memory layout constants (can be overridden by CMake for different platforms)
#ifndef FORTH_MEMORY_SIZE
#define FORTH_MEMORY_SIZE (64 * 1024)  // 64KB virtual memory (default)
#endif
#define DATA_STACK_SIZE 64             // Per standard minimum
#define RETURN_STACK_SIZE 48           // Per standard minimum
#define INPUT_BUFFER_SIZE 256          // Text input buffer

// Word structure
typedef struct word {
    struct word* link;          // Link to previous word (C pointer)
    char name[32];              // Word name (31 chars max per standard)
    uint8_t flags;              // Immediate flag, etc.
    void (*cfunc)(struct word* self);  // C function for ALL word types
    // Parameter field follows immediately after this structure
} word_t;

// Word flags
#define WORD_FLAG_IMMEDIATE 0x01

// Global memory
extern uint8_t forth_memory[FORTH_MEMORY_SIZE];
extern forth_addr_t here;  // Data space pointer

// Basic memory management functions
forth_addr_t forth_allot(size_t bytes);
void forth_align(void);
word_t* addr_to_word(forth_addr_t addr);
forth_addr_t word_to_addr(word_t* word);

#endif // FORTH_H