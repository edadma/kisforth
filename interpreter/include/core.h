#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"

#define FORTH_PAD_SIZE 1024

// Global memory
extern cell_t* state_ptr;  // C pointer to STATE variable for efficiency
extern cell_t* base_ptr;  // BASE variable pointer

extern forth_addr_t current_ip;

void create_builtin_definitions(void);

// Colon definition words
void f_colon(word_t* self);        // : ( C: "<spaces>name" -- colon-sys )
void f_semicolon(word_t* self);    // ; ( C: colon-sys -- )
void f_exit(word_t* self);         // EXIT ( -- ) ( R: nest-sys -- )

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
void f_find(word_t* self);          // FIND ( c-addr -- c-addr 0 | xt 1 | xt -1 )

void f_debug_on(word_t* self);      // DEBUG-ON ( -- )
void f_debug_off(word_t* self);     // DEBUG-OFF ( -- )

void f_create(word_t* self);
void f_variable(word_t* self);
void f_unused(word_t* self);

// DO/LOOP runtime primitive functions
void f_do_runtime(word_t* self);        // (DO) ( limit start -- ) ( R: -- loop-sys )
void f_loop_runtime(word_t* self);      // (LOOP) ( -- ) ( R: loop-sys1 -- | loop-sys2 )
void f_plus_loop_runtime(word_t* self); // (+LOOP) ( n -- ) ( R: loop-sys1 -- | loop-sys2 )
void f_i(word_t* self);                 // I ( -- n ) ( R: loop-sys -- loop-sys )
void f_j(word_t* self);                 // J ( -- n ) ( R: loop-sys1 loop-sys2 -- loop-sys1 loop-sys2 )
void f_leave_runtime(word_t* self);     // (LEAVE) ( -- ) ( R: loop-sys -- )
void f_unloop(word_t* self);            // UNLOOP ( -- ) ( R: loop-sys -- )

// DO/LOOP immediate compilation words
void f_do(word_t* self);                // DO ( C: -- do-sys )
void f_loop(word_t* self);              // LOOP ( C: do-sys -- )
void f_plus_loop(word_t* self);         // +LOOP ( C: do-sys -- )
void f_leave(word_t* self);             // LEAVE ( C: -- )

void create_all_primitives(void); // Create all primitive words

#endif // CORE_H