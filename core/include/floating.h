#ifndef FLOATING_H
#define FLOATING_H

#ifdef FORTH_ENABLE_FLOATING

// Forward declaration (word_t defined in forth.h)
struct word;

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

// Float parsing
bool try_parse_float(const char* token, double* result);

// Initialization functions
void create_floating_primitives(void);
void create_floating_definitions(void);

// Primitive implementations (initial set)
void f_fdrop(struct word* self);     // FDROP ( F: r -- )
void f_f_plus(struct word* self);    // F+ ( F: r1 r2 -- r3 )
void f_f_dot(struct word* self);     // F. ( F: r -- )

#endif // FORTH_ENABLE_FLOATING
#endif // FLOATING_H