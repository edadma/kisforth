#ifndef FLOATING_H
#define FLOATING_H

#ifdef FORTH_ENABLE_FLOATING

#include <stdbool.h>

#include "types.h"

// Float stack
#define FLOAT_STACK_SIZE 32
extern double float_stack[FLOAT_STACK_SIZE];
extern int float_stack_ptr;

// Float stack operations
void float_stack_init(void);
void float_push(double value);
double float_pop(void);
double float_peek(void);
int float_depth(void);

void compile_float_literal(double value);

// Float parsing
bool try_parse_float(const char* token, double* result);

// Initialization functions
void create_floating_primitives(void);
void create_floating_definitions(void);

// Primitive implementations
void f_fdrop(word_t* self);       // FDROP ( F: r -- )
void f_fdup(word_t* self);        // FDUP ( F: r -- r r )
void f_f_plus(word_t* self);      // F+ ( F: r1 r2 -- r3 )
void f_f_minus(word_t* self);     // F- ( F: r1 r2 -- r3 )
void f_f_multiply(word_t* self);  // F* ( F: r1 r2 -- r3 )
void f_f_divide(word_t* self);    // F/ ( F: r1 r2 -- r3 )
void f_f_dot(word_t* self);       // F. ( F: r -- )
void f_flit(word_t* self);        // FLIT ( F: -- r ) [float value follows]

#endif  // FORTH_ENABLE_FLOATING
#endif  // FLOATING_H