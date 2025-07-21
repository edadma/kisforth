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

#endif  // FORTH_ENABLE_FLOATING
#endif  // FLOATING_H