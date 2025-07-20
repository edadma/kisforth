#ifndef FLOATING_H
#define FLOATING_H

#ifdef FORTH_ENABLE_FLOATING

#include <stdbool.h>

#include "forth.h"

// Float stack
#define FLOAT_STACK_SIZE 32
extern double float_stack[FLOAT_STACK_SIZE];
extern int float_stack_ptr;

// Float stack operations
void float_stack_init(void);
void float_push(context_t* ctx, double value);
double float_pop(context_t* ctx);
double float_peek(context_t* ctx);
int float_depth(context_t* ctx);

void compile_float_literal(context_t* ctx, double value);

// Float parsing
bool try_parse_float(const char* token, double* result);

// Initialization functions
void create_floating_primitives(void);
void create_floating_definitions(void);

// Primitive implementations
void f_fdrop(context_t* ctx, word_t* self);      // FDROP ( F: r -- )
void f_fdup(context_t* ctx, word_t* self);       // FDUP ( F: r -- r r )
void f_fplus(context_t* ctx, word_t* self);      // F+ ( F: r1 r2 -- r3 )
void f_fminus(context_t* ctx, word_t* self);     // F- ( F: r1 r2 -- r3 )
void f_fmultiply(context_t* ctx, word_t* self);  // F* ( F: r1 r2 -- r3 )
void f_fdivide(context_t* ctx, word_t* self);    // F/ ( F: r1 r2 -- r3 )
void f_fdot(context_t* ctx, word_t* self);       // F. ( F: r -- )
void f_flit(context_t* ctx,
            word_t* self);  // FLIT ( F: -- r ) [float value follows]

#endif  // FORTH_ENABLE_FLOATING
#endif  // FLOATING_H