#ifndef CORE_H
#define CORE_H

#include "context.h"
#include "types.h"

#define FORTH_PAD_SIZE 1024

// Global memory
extern cell_t* state_ptr;  // C pointer to STATE variable for efficiency
extern cell_t* base_ptr;   // BASE variable pointer

extern forth_addr_t current_ip;

void create_builtin_definitions(void);

// Colon definition words
void f_colon(context_t* ctx,
             word_t* self);  // : ( C: "<spaces>name" -- colon-sys )
void f_semicolon(context_t* ctx, word_t* self);  // ; ( C: colon-sys -- )
void f_exit(context_t* ctx, word_t* self);  // EXIT ( -- ) ( R: nest-sys -- )

// Primitive word implementations
void f_plus(context_t* ctx, word_t* self);
void f_minus(context_t* ctx, word_t* self);
void f_multiply(context_t* ctx, word_t* self);
void f_divide(context_t* ctx, word_t* self);
void f_drop(context_t* ctx, word_t* self);
void f_source(context_t* ctx, word_t* self);       // SOURCE ( -- c-addr u )
void f_to_in(context_t* ctx, word_t* self);        // >IN ( -- addr )
void f_dot(context_t* ctx, word_t* self);          // . ( n -- )
void f_store(context_t* ctx, word_t* self);        // ! ( x addr -- )
void f_fetch(context_t* ctx, word_t* self);        // @ ( addr -- x )
void f_c_store(context_t* ctx, word_t* self);      // C! ( char addr -- )
void f_c_fetch(context_t* ctx, word_t* self);      // C@ ( addr -- char )
void f_equals(context_t* ctx, word_t* self);       // = ( x1 x2 -- flag )
void f_less_than(context_t* ctx, word_t* self);    // < ( x1 x2 -- flag )
void f_zero_equals(context_t* ctx, word_t* self);  // 0= ( x -- flag )
void f_swap(context_t* ctx, word_t* self);         // SWAP ( x1 x2 -- x2 x1 )
void f_rot(context_t* ctx, word_t* self);  // ROT ( x1 x2 x3 -- x2 x3 x1 )
void f_pick(context_t* ctx,
            word_t* self);  // PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )
void f_here(context_t* ctx, word_t* self);   // HERE ( -- addr )
void f_allot(context_t* ctx, word_t* self);  // ALLOT ( n -- )
void f_comma(context_t* ctx, word_t* self);  // , ( x -- )
void f_lit(context_t* ctx, word_t* self);    // LIT ( -- x ) [value follows]
void f_sm_rem(
    context_t* ctx,
    word_t* self);  // SM/REM ( d1 n1 -- n2 n3 )  Symmetric division primitive
void f_fm_mod(
    context_t* ctx,
    word_t* self);  // FM/MOD ( d1 n1 -- n2 n3 )  Floored division primitive
void f_and(context_t* ctx, word_t* self);        // AND ( x1 x2 -- x3 )
void f_or(context_t* ctx, word_t* self);         // OR ( x1 x2 -- x3 )
void f_xor(context_t* ctx, word_t* self);        // XOR ( x1 x2 -- x3 )
void f_invert(context_t* ctx, word_t* self);     // INVERT ( x1 -- x2 )
void f_emit(context_t* ctx, word_t* self);       // EMIT ( char -- )
void f_key(context_t* ctx, word_t* self);        // KEY ( -- char )
void f_type(context_t* ctx, word_t* self);       // TYPE ( c-addr u -- )
void f_to_r(context_t* ctx, word_t* self);       // >R ( x -- ) ( R: -- x )
void f_r_from(context_t* ctx, word_t* self);     // R> ( -- x ) ( R: x -- )
void f_r_fetch(context_t* ctx, word_t* self);    // R@ ( -- x ) ( R: x -- x )
void f_m_star(context_t* ctx, word_t* self);     // M* ( n1 n2 -- d )
void f_immediate(context_t* ctx, word_t* self);  // IMMEDIATE ( -- )
void f_roll(context_t* ctx, word_t* self);
void f_display_counted_string(context_t* ctx, word_t* self);
void f_dot_quote(context_t* ctx, word_t* self);
void f_dot_quote_runtime(context_t* ctx, word_t* self);
void f_abort_quote(context_t* ctx, word_t* self);
void f_abort_quote_runtime(context_t* ctx, word_t* self);
void f_bracket_tick(context_t* ctx, word_t* self);
void f_0branch(context_t* ctx, word_t* self);
void f_branch(context_t* ctx, word_t* self);
void f_u_less(context_t* ctx, word_t* self);
void f_tick(context_t* ctx, word_t* self);     // ' ( "<spaces>name" -- xt )
void f_execute(context_t* ctx, word_t* self);  // EXECUTE ( i*x xt -- j*x )
void f_find(context_t* ctx,
            word_t* self);  // FIND ( c-addr -- c-addr 0 | xt 1 | xt -1 )

void f_create(context_t* ctx, word_t* self);
void f_variable(context_t* ctx, word_t* self);
void f_unused(context_t* ctx, word_t* self);

// DO/LOOP runtime primitive functions
void f_do_runtime(context_t* ctx,
                  word_t* self);  // (DO) ( limit start -- ) ( R: -- loop-sys )
void f_loop_runtime(
    context_t* ctx,
    word_t* self);  // (LOOP) ( -- ) ( R: loop-sys1 -- | loop-sys2 )
void f_plus_loop_runtime(
    context_t* ctx,
    word_t* self);  // (+LOOP) ( n -- ) ( R: loop-sys1 -- | loop-sys2 )
void f_i(context_t* ctx,
         word_t* self);  // I ( -- n ) ( R: loop-sys -- loop-sys )
void f_j(context_t* ctx, word_t* self);  // J ( -- n ) ( R: loop-sys1 loop-sys2
                                         // -- loop-sys1 loop-sys2 )
void f_leave_runtime(context_t* ctx,
                     word_t* self);  // (LEAVE) ( -- ) ( R: loop-sys -- )
void f_unloop(context_t* ctx,
              word_t* self);  // UNLOOP ( -- ) ( R: loop-sys -- )

// DO/LOOP immediate compilation words
void f_do(context_t* ctx, word_t* self);         // DO ( C: -- do-sys )
void f_loop(context_t* ctx, word_t* self);       // LOOP ( C: do-sys -- )
void f_plus_loop(context_t* ctx, word_t* self);  // +LOOP ( C: do-sys -- )
void f_leave(context_t* ctx, word_t* self);      // LEAVE ( C: -- )

void f_word(context_t* ctx,
            word_t* self);  // WORD ( char "<chars>cchar<chars>" -- c-addr )
void f_accept(context_t* ctx, word_t* self);  // ACCEPT ( c-addr +n1 -- +n2 )

void f_s_quote_runtime(context_t* ctx, word_t* self);
void f_s_quote(context_t* ctx, word_t* self);

void create_all_primitives(void);  // Create all primitive words

#endif  // CORE_H