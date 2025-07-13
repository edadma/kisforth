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
cell_t data_peek_at(int offset);  // Peek at stack[depth-1-offset]
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

// Input buffer management (ANS Forth standard)
extern char input_buffer[INPUT_BUFFER_SIZE];
extern cell_t input_length;
extern cell_t to_in;  // >IN variable - current parse position

void set_input_buffer(const char* text);
char* source_addr(void);
cell_t source_length(void);

// Text interpreter (ANS Forth compliant)
void skip_spaces(void);
char* parse_name(char* dest, size_t max_len);
bool try_parse_number(const char* token, cell_t* result);
void interpret(void);  // Standard interpreter loop

// Word creation and execution
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self));
void execute_word(word_t* word);

// Primitive word implementations
void f_plus(word_t* self);
void f_minus(word_t* self);
void f_multiply(word_t* self);
void f_divide(word_t* self);
void f_drop(word_t* self);
void f_source(word_t* self);    // SOURCE ( -- c-addr u )
void f_to_in(word_t* self);     // >IN ( -- addr )

// I/O interface - platform abstraction
typedef struct {
    void (*print_string)(const char* str);
    void (*print_char)(char c);
    char* (*read_line)(const char* prompt);
    void (*cleanup)(void);
} io_interface_t;

extern io_interface_t* current_io;

// I/O interface management
void set_io_interface(io_interface_t* io);
void io_print(const char* str);
void io_print_char(char c);
char* io_read_line(const char* prompt);
void io_cleanup(void);

// REPL system
void forth_repl(void);
void f_quit(word_t* self);
void f_bye(word_t* self);

// Platform-specific I/O interfaces
io_interface_t* get_pc_io(void);  // PC implementation

// System management
void forth_reset(void);         // Complete system reset
void create_all_primitives(void); // Create all primitive words

// Unit testing system
#ifdef FORTH_ENABLE_TESTS

typedef struct {
    int passed;
    int failed;
    int total;
    const char* current_test_name;
} test_stats_t;

extern test_stats_t test_stats;

// Self-contained assertions
#define TEST_ASSERT_TRUE(condition) \
    do { \
        test_stats.total++; \
        if (condition) { \
            test_stats.passed++; \
        } else { \
            test_stats.failed++; \
            printf("    FAIL: %s (line %d) in %s\n", #condition, __LINE__, \
                   test_stats.current_test_name ? test_stats.current_test_name : "unknown"); \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        test_stats.total++; \
        if ((expected) == (actual)) { \
            test_stats.passed++; \
        } else { \
            test_stats.failed++; \
            printf("    FAIL: expected %d, got %d (line %d) in %s\n", \
                   (int)(expected), (int)(actual), __LINE__, \
                   test_stats.current_test_name ? test_stats.current_test_name : "unknown"); \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    TEST_ASSERT_TRUE((ptr) != NULL)

#define TEST_ASSERT_STACK_DEPTH(expected) \
    TEST_ASSERT_EQUAL(expected, data_depth())

#define TEST_ASSERT_STACK_TOP(expected) \
    TEST_ASSERT_EQUAL(expected, data_peek())

#define TEST_FORTH(name, code, expected_top, expected_depth) \
    do { \
        printf("  %s: \"%s\"", name, code); \
        int old_failed = test_stats.failed; \
        test_stats.current_test_name = name; \
        test_forth_code(code, expected_top, expected_depth); \
        printf(" -> %s\n", test_stats.failed == old_failed ? "PASS" : "FAIL"); \
    } while(0)

#define TEST_FUNC(name, func) \
    do { \
        printf("  %s", name); \
        int old_failed = test_stats.failed; \
        test_stats.current_test_name = name; \
        func(); \
        printf(" -> %s\n", test_stats.failed == old_failed ? "PASS" : "FAIL"); \
    } while(0)

// Test functions
bool test_forth_code(const char* code, cell_t expected_top, int expected_depth);
void run_all_tests(void);
void f_test(word_t* self);

#else
// Compiled out when tests disabled
#define TEST_ASSERT_TRUE(condition) do {} while(0)
#define TEST_ASSERT_EQUAL(expected, actual) do {} while(0)
#define TEST_ASSERT_NOT_NULL(ptr) do {} while(0)
#define TEST_ASSERT_STACK_DEPTH(expected) do {} while(0)
#define TEST_ASSERT_STACK_TOP(expected) do {} while(0)
#define TEST_FORTH(name, code, expected_top, expected_depth) do {} while(0)
#define TEST_FUNC(name, func) do {} while(0)

// Stub functions
static inline void f_test(word_t* self) {
    printf("Tests not compiled in. Rebuild with -DENABLE_TESTS=ON\n");
}
#endif

#endif // FORTH_H