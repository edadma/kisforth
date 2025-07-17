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

#define FORTH_PAD_SIZE 1024

#define require(condition, ...) \
    do { \
        if (!(condition)) { \
            error("Requirement failed: %s at %s:%d - " __VA_ARGS__, \
                  #condition, __FILE__, __LINE__); \
        } \
    } while(0)

// Word structure
typedef struct word {
    struct word* link;          // Link to previous word (C pointer)
    char name[32];              // Word name (31 chars max per standard)
    uint32_t flags;             // Immediate flag, etc.
    void (*cfunc)(struct word* self);  // C function for ALL word types
    // DUAL-PURPOSE PARAMETER FIELD:
    // - For most words: Forth address pointing to parameter space
    // - For variables:  Direct storage of the variable's value
    uint32_t param_field;       // either value or Forth address
} word_t;

// Word flags
#define WORD_FLAG_IMMEDIATE 0x01

// Global memory
extern uint8_t forth_memory[FORTH_MEMORY_SIZE];
extern forth_addr_t here;  // Data space pointer
extern cell_t* state_ptr;  // C pointer to STATE variable for efficiency
extern cell_t* base_ptr;  // BASE variable pointer

// Stack structures
extern cell_t data_stack[DATA_STACK_SIZE];
extern cell_t return_stack[RETURN_STACK_SIZE];
extern int data_stack_ptr;    // Points to next empty slot
extern int return_stack_ptr;  // Points to next empty slot

extern forth_addr_t current_ip;

uintptr_t align_up(uintptr_t addr, size_t alignment);

void error(const char* format, ...);

char digit_to_char(int digit);
int char_to_digit(char c, int base);
void print_number_in_base(cell_t value, cell_t base);

// Test accessor functions for Forth memory input system:
cell_t get_current_to_in(void);
cell_t get_current_input_length(void);
forth_addr_t get_current_input_buffer_addr(void);

void set_input_buffer(const char* text);  // Set input buffer content

// Basic memory management functions
forth_addr_t forth_allot(size_t bytes);
void forth_align(void);
word_t* addr_to_ptr(forth_addr_t addr);
forth_addr_t ptr_to_addr(word_t* word);

// Memory access functions (needed for @, !, C@, C!)
void forth_store(forth_addr_t addr, cell_t value);    // !
cell_t forth_fetch(forth_addr_t addr);                // @
void forth_c_store(forth_addr_t addr, byte_t value);  // C!
byte_t forth_c_fetch(forth_addr_t addr);              // C@

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
word_t* search_word(const char* name);
void show_dictionary(void);  // Debug helper

// Input buffer management (ANS Forth standard)
void input_system_init(void);
extern forth_addr_t input_buffer_addr;
extern forth_addr_t to_in_addr;
extern forth_addr_t input_length_addr;

// Text interpreter (ANS Forth compliant)
void skip_spaces(void);
char* parse_name(char* dest, size_t max_len);
bool try_parse_number(const char* token, cell_t* result);
void interpret(void);  // Standard interpreter loop
void interpret_text(const char* text);  // Convenience: set_input_buffer + interpret

void create_builtin_definitions(void);

// Word creation and execution
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self));
cell_t* create_variable_word(const char* name, cell_t initial_value);
void create_area_word(const char* name);
word_t* create_immediate_primitive_word(const char* name, void (*cfunc)(word_t* self));
void execute_word(word_t* word);
word_t* defining_word(void (*cfunc)(struct word* self));

// Compilation support
void compile_token(forth_addr_t token);
void compile_literal(cell_t value);

// Colon definition words
void f_colon(word_t* self);        // : ( C: "<spaces>name" -- colon-sys )
void f_semicolon(word_t* self);    // ; ( C: colon-sys -- )
void f_exit(word_t* self);         // EXIT ( -- ) ( R: nest-sys -- )
void execute_colon(word_t* self);  // Generic colon definition executor

// Primitive word implementations
void f_plus(word_t* self);
void f_minus(word_t* self);
void f_multiply(word_t* self);
void f_divide(word_t* self);
void f_drop(word_t* self);
void f_source(word_t* self);    // SOURCE ( -- c-addr u )
void f_to_in(word_t* self);     // >IN ( -- addr )
void f_dot(word_t* self);           // . ( n -- )
void f_store(word_t* self);      // ! ( x addr -- )
void f_fetch(word_t* self);      // @ ( addr -- x )
void f_c_store(word_t* self);    // C! ( char addr -- )
void f_c_fetch(word_t* self);    // C@ ( addr -- char )
void f_equals(word_t* self);     // = ( x1 x2 -- flag )
void f_less_than(word_t* self);  // < ( x1 x2 -- flag )
void f_zero_equals(word_t* self); // 0= ( x -- flag )
void f_swap(word_t* self);          // SWAP ( x1 x2 -- x2 x1 )
void f_rot(word_t* self);           // ROT ( x1 x2 x3 -- x2 x3 x1 )
void f_pick(word_t* self);          // PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )
void f_here(word_t* self);           // HERE ( -- addr )
void f_allot(word_t* self);          // ALLOT ( n -- )
void f_comma(word_t* self);          // , ( x -- )
void f_lit(word_t* self);            // LIT ( -- x ) [value follows]
void f_sm_rem(word_t* self);	// SM/REM ( d1 n1 -- n2 n3 )  Symmetric division primitive
void f_fm_mod(word_t* self);	// FM/MOD ( d1 n1 -- n2 n3 )  Floored division primitive
void f_and(word_t* self);        // AND ( x1 x2 -- x3 )
void f_or(word_t* self);         // OR ( x1 x2 -- x3 )
void f_xor(word_t* self);        // XOR ( x1 x2 -- x3 )
void f_invert(word_t* self);     // INVERT ( x1 -- x2 )
void f_emit(word_t* self);       // EMIT ( char -- )
void f_key(word_t* self);        // KEY ( -- char )
void f_type(word_t* self);       // TYPE ( c-addr u -- )
void f_to_r(word_t* self);      // >R ( x -- ) ( R: -- x )
void f_r_from(word_t* self);    // R> ( -- x ) ( R: x -- )
void f_r_fetch(word_t* self);   // R@ ( -- x ) ( R: x -- x )
void f_m_star(word_t* self);    // M* ( n1 n2 -- d )
void f_address(word_t* self);       // Variable execution ( -- addr )
void f_immediate(word_t* self);      // IMMEDIATE ( -- )
void f_roll(word_t* self);
void f_display_counted_string(word_t* self);
void f_dot_quote(word_t* self);
void f_dot_quote_runtime(word_t* self);
void f_abort_quote(word_t* self);
void f_abort_quote_runtime(word_t* self);
void f_bracket_tick(word_t* self);
void f_0branch(word_t* self);
void f_branch(word_t* self);
void f_u_less(word_t* self);
void f_tick(word_t* self);          // ' ( "<spaces>name" -- xt )
void f_execute(word_t* self);       // EXECUTE ( i*x xt -- j*x )

// I/O interface - platform abstraction
typedef struct {
    void (*print_string)(const char* str);
    void (*print_char)(char c);
    char* (*read_line)(const char* prompt);
    void (*cleanup)(void);
} io_interface_t;

// REPL system
void forth_repl(void);
void f_quit(word_t* self);
void f_abort(word_t* self);
void f_bye(word_t* self);

void f_debug_on(word_t* self);      // DEBUG-ON ( -- )
void f_debug_off(word_t* self);     // DEBUG-OFF ( -- )

void f_create(word_t* self);
void f_variable(word_t* self);
void f_param_field(word_t* self);

// System management
void forth_reset(void);         // Complete system reset
void create_all_primitives(void); // Create all primitive words

int parse_string(char quote_char, char* dest, size_t max_len);
void set_current_to_in(cell_t value);
forth_addr_t store_counted_string(const char* str, int length);


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