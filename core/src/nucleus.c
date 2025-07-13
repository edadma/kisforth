#include "forth.h"
#include <string.h>
#include <assert.h>

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
    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 + n2);
}

// - ( n1 n2 -- n3 )  Subtract n2 from n1, leaving difference n3
void f_minus(word_t* self) {
    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 - n2);
}

// * ( n1 n2 -- n3 )  Multiply n1 by n2, leaving product n3
void f_multiply(word_t* self) {
    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    data_push(n1 * n2);
}

// / ( n1 n2 -- n3 )  Divide n1 by n2, leaving quotient n3
void f_divide(word_t* self) {
    cell_t n2 = data_pop();
    cell_t n1 = data_pop();
    assert(n2 != 0);  // Division by zero check
    data_push(n1 / n2);
}

// DROP ( x -- )  Remove x from the stack
void f_drop(word_t* self) {
    data_pop();
}

// SOURCE ( -- c-addr u )  Return input buffer address and length
void f_source(word_t* self) {
    data_push((cell_t)source_addr());  // Address of input buffer
    data_push(source_length());        // Length of input buffer
}

// >IN ( -- addr )  Return address of >IN variable
void f_to_in(word_t* self) {
    data_push((cell_t)&to_in);  // Address of >IN variable
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

    #ifdef FORTH_ENABLE_TESTS
    create_primitive_word("TEST", f_test);
    #endif
}