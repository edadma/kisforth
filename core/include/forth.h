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

// Stack structures
extern cell_t data_stack[DATA_STACK_SIZE];
extern cell_t return_stack[RETURN_STACK_SIZE];
extern int data_stack_ptr;    // Points to next empty slot
extern int return_stack_ptr;  // Points to next empty slot

// Basic memory management functions
forth_addr_t forth_allot(size_t bytes);
void forth_align(void);
word_t* addr_to_word(forth_addr_t addr);
forth_addr_t word_to_addr(word_t* word);

// Stack operations
void stack_init(void);
void data_push(cell_t value);
cell_t data_pop(void);
cell_t data_peek(void);
int data_depth(void);
void return_push(cell_t value);
cell_t return_pop(void);
int return_depth(void);

// Dictionary management
extern word_t* dictionary_head;  // Points to most recently defined word

void dictionary_init(void);
void link_word(word_t* word);
word_t* find_word(const char* name);
void show_dictionary(void);  // Debug helper

// Word creation and execution
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self));
void execute_word(word_t* word);

// Primitive word implementations
void f_plus(word_t* self);
void f_minus(word_t* self);
void f_multiply(word_t* self);
void f_divide(word_t* self);
void f_drop(word_t* self);

#endif // FORTH_H