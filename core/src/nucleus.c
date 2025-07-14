#include "forth.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

// Create a primitive word in virtual memory
word_t* create_primitive_word(const char* name, void (*cfunc)(word_t* self)) {
    // Align to word boundary
    forth_align();

    // Allocate space for the word structure
    forth_addr_t word_addr = forth_allot(sizeof(word_t));
    word_t* word = addr_to_word(word_addr);

    // Initialize the word structure
    word->link = NULL;  // Will be set by link_word()
    strncpy(word->name, name, sizeof(word->name) - 1);
    word->name[sizeof(word->name) - 1] = '\0';  // Ensure null termination
    word->flags = 0;
    word->cfunc = cfunc;

    // Primitives have no parameter field (zero bytes allocated)

    // Automatically link into dictionary
    link_word(word);

    return word;
}

// Execute a word by calling its cfunc
void execute_word(word_t* word) {
    assert(word != NULL);
    assert(word->cfunc != NULL);
    word->cfunc(word);
}

// Arithmetic primitives - these operate on the data stack

// + ( n1 n2 -- n3 )  Add n1 and n2, leaving sum n3
void f_plus(word_t* self) {
    (void)self;

    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 + n2);
}

// - ( n1 n2 -- n3 )  Subtract n2 from n1, leaving difference n3
void f_minus(word_t* self) {
    (void)self;

    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 - n2);
}

// * ( n1 n2 -- n3 )  Multiply n1 by n2, leaving product n3
void f_multiply(word_t* self) {
    (void)self;

    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 * n2);
}

// / ( n1 n2 -- n3 )  Divide n1 by n2, leaving quotient n3
void f_divide(word_t* self) {
    (void)self;

    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    assert(n2 != 0);  // Division by zero check
    data_push(n1 / n2);
}

// DROP ( x -- )  Remove x from the stack
void f_drop(word_t* self) {
    (void)self;

    data_pop();
}

// SOURCE ( -- c-addr u )  Return input buffer address and length
void f_source(word_t* self) {
    (void)self;

    data_push(input_buffer_addr);      // Forth address
    data_push(forth_fetch(input_length_addr));       // Current length from Forth memory
}

// >IN ( -- addr )  Return address of >IN variable
void f_to_in(word_t* self) {
    (void)self;

    data_push(to_in_addr);             // Forth address of >IN
}

// . ( n -- ) Print and remove top stack item
void f_dot(word_t* self) {
    (void)self;

    //if (data_depth() == 0) {
    //    forth_abort("Stack underflow");
    //    return;
    //}

    cell_t value = data_pop();
    printf("%d ", value);  // Print with trailing space per ANS standard
    fflush(stdout);        // Ensure immediate output
}

// ! ( x addr -- )  Store x at addr
void f_store(word_t* self) {
    (void)self;

    forth_addr_t addr = (forth_addr_t)data_pop();
    cell_t value = data_pop();
    forth_store(addr, value);
}

// @ ( addr -- x )  Fetch value from addr
void f_fetch(word_t* self) {
    (void)self;

    forth_addr_t addr = (forth_addr_t)data_pop();
    cell_t value = forth_fetch(addr);
    data_push(value);
}

// C! ( char addr -- )  Store char at addr
void f_c_store(word_t* self) {
    (void)self;

    forth_addr_t addr = (forth_addr_t)data_pop();
    byte_t value = (byte_t)data_pop();
    forth_c_store(addr, value);
}

// C@ ( addr -- char )  Fetch char from addr
void f_c_fetch(word_t* self) {
    (void)self;

    forth_addr_t addr = (forth_addr_t)data_pop();
    byte_t value = forth_c_fetch(addr);
    data_push((cell_t)value);
}

// Comparison primitives - these operate on the data stack and return ANS Forth flags

// = ( x1 x2 -- flag )  Return true if x1 equals x2
void f_equals(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();

    // ANS Forth: true = -1, false = 0
    cell_t flag = (x1 == x2) ? -1 : 0;
    data_push(flag);
}

// < ( x1 x2 -- flag )  Return true if x1 is less than x2 (signed comparison)
void f_less_than(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();

    // ANS Forth: true = -1, false = 0
    cell_t flag = (x1 < x2) ? -1 : 0;
    data_push(flag);
}

// 0= ( x -- flag )  Return true if x equals zero
void f_zero_equals(word_t* self) {
    (void)self;

    cell_t x = data_pop();

    // ANS Forth: true = -1, false = 0
    cell_t flag = (x == 0) ? -1 : 0;
    data_push(flag);
}

// Create all primitive words - called during system initialization
void create_all_primitives(void) {
    create_primitive_word("+", f_plus);
    create_primitive_word("-", f_minus);
    create_primitive_word("*", f_multiply);
    create_primitive_word("/", f_divide);
    create_primitive_word("DROP", f_drop);
    create_primitive_word("SOURCE", f_source);
    create_primitive_word(">IN", f_to_in);
    create_primitive_word("QUIT", f_quit);
    create_primitive_word("BYE", f_bye);
    create_primitive_word(".", f_dot);
    create_primitive_word("!", f_store);
    create_primitive_word("@", f_fetch);
    create_primitive_word("C!", f_c_store);
    create_primitive_word("C@", f_c_fetch);
    create_primitive_word("=", f_equals);
    create_primitive_word("<", f_less_than);
    create_primitive_word("0=", f_zero_equals);

	#ifdef FORTH_DEBUG_ENABLED
    create_primitive_word("DEBUG-ON", f_debug_on);
    create_primitive_word("DEBUG-OFF", f_debug_off);
	#endif

    #ifdef FORTH_ENABLE_TESTS
    create_primitive_word("TEST", f_test);
    #endif
}