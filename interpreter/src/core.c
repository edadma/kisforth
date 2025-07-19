#include "core.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "dictionary.h"
#include "error.h"
#include "forth.h"
#include "memory.h"
#include "repl.h"
#include "stack.h"
#include "text.h"
#include "util.h"

// Global STATE pointer for efficient access
cell_t* state_ptr = NULL;

// Global BASE pointer for efficient access
cell_t* base_ptr = NULL;

#define MAX_LOOP_LEAVES 32
#define MAX_NESTED_LOOPS 8

typedef struct {
  forth_addr_t loop_start_addr;  // Address for backward branch from LOOP
  forth_addr_t leave_addrs[MAX_LOOP_LEAVES];  // Forward branches from LEAVE
  int leave_count;                            // Number of pending LEAVEs
} loop_frame_t;

// Loop compilation stack
static loop_frame_t loop_stack[MAX_NESTED_LOOPS];
static int loop_stack_depth = 0;

// Loop compilation stack management functions
static void push_loop_frame(context_t* ctx, forth_addr_t start_addr) {
  if (loop_stack_depth >= MAX_NESTED_LOOPS) {
    error(ctx, "Loop nesting too deep (max %d)", MAX_NESTED_LOOPS);
  }

  loop_frame_t* frame = &loop_stack[loop_stack_depth];
  frame->loop_start_addr = start_addr;
  frame->leave_count = 0;
  loop_stack_depth++;

  debug("Push loop frame: depth=%d, start_addr=%d", loop_stack_depth,
        start_addr);
}

static loop_frame_t* current_loop_frame(context_t* ctx) {
  if (loop_stack_depth == 0) {
    error(ctx, "No active loop for LEAVE");
  }
  return &loop_stack[loop_stack_depth - 1];
}

static void add_leave_addr(context_t* ctx, forth_addr_t leave_addr) {
  loop_frame_t* frame = current_loop_frame(ctx);

  if (frame->leave_count >= MAX_LOOP_LEAVES) {
    error(ctx, "Too many LEAVE statements in loop (max %d)", MAX_LOOP_LEAVES);
  }

  frame->leave_addrs[frame->leave_count++] = leave_addr;
  debug("Add LEAVE addr: %d (count now %d)", leave_addr, frame->leave_count);
}

static loop_frame_t pop_loop_frame(context_t* ctx) {
  if (loop_stack_depth == 0) {
    error(ctx, "No active loop to close");
  }

  loop_frame_t frame = loop_stack[--loop_stack_depth];
  debug("Pop loop frame: depth now %d, had %d LEAVEs", loop_stack_depth,
        frame.leave_count);
  return frame;
}

// Arithmetic primitives - these operate on the data stack

// + ( n1 n2 -- n3 )  Add n1 and n2, leaving sum n3
void f_plus(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);
  data_push(ctx, n1 + n2);
}

// - ( n1 n2 -- n3 )  Subtract n2 from n1, leaving difference n3
void f_minus(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);
  data_push(ctx, n1 - n2);
}

// * ( n1 n2 -- n3 )  Multiply n1 by n2, leaving product n3
void f_multiply(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);
  data_push(ctx, n1 * n2);
}

// / ( n1 n2 -- n3 )  Divide n1 by n2, leaving quotient n3
void f_divide(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);

  if (n2 == 0) error(ctx, "Division by zero in '/'");

  data_push(ctx, n1 / n2);
}

// DROP ( x -- )  Remove x from the stack
void f_drop(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  data_pop(ctx);
}

// SOURCE ( -- c-addr u )  Return input buffer address and length
void f_source(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  data_push(ctx, input_buffer_addr);  // Forth address
  data_push(
      ctx,
      forth_fetch(input_length_addr));  // Current length from Forth memory
}

// >IN ( -- addr )  Return address of >IN variable
void f_to_in(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  data_push(ctx, to_in_addr);  // Forth address of >IN
}

// . ( n -- ) Print and remove top stack item (BASE-aware)
void f_dot(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t value = data_pop(ctx);
  cell_t base = *base_ptr;

  // Validate base range, fall back to decimal if invalid
  if (base < 2 || base > 36) {
    base = 10;
  }

  print_number_in_base(value, base);
  putchar(' ');
  fflush(stdout);
}

// ! ( x addr -- )  Store x at addr
void f_store(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t addr = (forth_addr_t)data_pop(ctx);
  cell_t value = data_pop(ctx);
  forth_store(addr, value);
}

// @ ( addr -- x )  Fetch value from addr
void f_fetch(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t addr = (forth_addr_t)data_pop(ctx);
  cell_t value = forth_fetch(addr);
  data_push(ctx, value);
}

// C! ( char addr -- )  Store char at addr
void f_c_store(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t addr = (forth_addr_t)data_pop(ctx);
  byte_t value = (byte_t)data_pop(ctx);
  forth_c_store(addr, value);
}

// C@ ( addr -- char )  Fetch char from addr
void f_c_fetch(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t addr = (forth_addr_t)data_pop(ctx);
  byte_t value = forth_c_fetch(addr);
  data_push(ctx, (cell_t)value);
}

// Comparison primitives - these operate on the data stack and return ANS Forth
// flags

// = ( x1 x2 -- flag )  Return true if x1 equals x2
void f_equals(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);

  // ANS Forth: true = -1, false = 0
  cell_t flag = (x1 == x2) ? -1 : 0;
  data_push(ctx, flag);
}

// < ( x1 x2 -- flag )  Return true if x1 is less than x2 (signed comparison)
void f_less_than(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);

  // ANS Forth: true = -1, false = 0
  cell_t flag = (x1 < x2) ? -1 : 0;
  data_push(ctx, flag);
}

// 0= ( x -- flag )  Return true if x equals zero
void f_zero_equals(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x = data_pop(ctx);

  // ANS Forth: true = -1, false = 0
  cell_t flag = (x == 0) ? -1 : 0;
  data_push(ctx, flag);
}

// SWAP ( x1 x2 -- x2 x1 )  Exchange the top two stack items
void f_swap(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);
  data_push(ctx, x2);
  data_push(ctx, x1);
}

// ROT ( x1 x2 x3 -- x2 x3 x1 )  Rotate third item to top
void f_rot(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x3 = data_pop(ctx);
  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);
  data_push(ctx, x2);
  data_push(ctx, x3);
  data_push(ctx, x1);
}

// PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )  Copy u-th stack item to top
// 0 PICK is equivalent to DUP, 1 PICK is equivalent to OVER
void f_pick(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t u = data_pop(ctx);

  // Bounds check: u must be >= 0 and < stack depth
  require(u >= 0);
  require(u < data_depth(ctx));

  // Use data_peek_at to get the u-th item from top
  cell_t xu = data_peek_at(ctx, u);
  data_push(ctx, xu);
}

// HERE ( -- addr )  Return the current data space pointer
void f_here(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  data_push(ctx, here);
}

// ALLOT ( n -- )  Allocate n bytes of data space
void f_allot(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n = data_pop(ctx);

  // Handle negative allot (deallocation) carefully
  if (n < 0) {
    // Negative allot - check bounds to prevent underflow
    require(here >= (forth_addr_t)(-n));
    here += n;  // n is negative, so this subtracts
  } else {
    // Positive allot - normal allocation
    forth_allot(n);
  }
}

// , ( x -- )  Store cell at HERE and advance HERE by one cell
void f_comma(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x = data_pop(ctx);

  // Align HERE to cell boundary before storing
  forth_align();

  // Store the value at current HERE
  forth_store(here, x);

  // Advance HERE by one cell
  here += sizeof(cell_t);
}

// : (colon) - start colon definition
// ( C: "<spaces>name" -- colon-sys )
void f_colon(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  word_t* word = defining_word(ctx, execute_colon);

  // Enter compilation state
  *state_ptr = -1;

  debug("Colon definition header created at %u, entering compilation mode",
        ptr_to_addr(word));
}

// ; (semicolon) - end colon definition
// Compilation: ( C: colon-sys -- )
void f_semicolon(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (*state_ptr == 0) error(ctx, "';' without matching :");

  debug("Ending colon definition, compiling EXIT");

  // Compile EXIT as the last token
  word_t* exit_word = find_word(ctx, "EXIT");

  compile_token(ptr_to_addr(exit_word));

  // Exit compilation state
  *state_ptr = 0;

  debug("Colon definition complete, exiting compilation mode");
}

// EXIT - return from colon definition using return stack
// Run-time: ( -- ) ( R: nest-sys -- )
void f_exit(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  debug("EXIT called");

  // Check if there's a saved instruction pointer on the return stack
  if (return_depth(ctx) > 0) {
    // Restore previous instruction pointer from return stack
    ctx->ip = (forth_addr_t)return_pop(ctx);
    debug("  Restored IP from return stack");
  } else {
    // No saved IP - we're at the top level, end execution
    ctx->ip = 0;
    debug("  No saved IP - ending execution");
  }
}

// LIT implementation that reads from instruction stream
// LIT ( -- x ) Push the literal value that follows in compiled code
void f_lit(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (ctx->ip == 0) error(ctx, "LIT called outside colon definition");

  // Read the literal value from the instruction stream
  cell_t literal = forth_fetch(ctx->ip);
  ctx->ip += sizeof(cell_t);  // Advance past the literal

  // Push the literal onto the data stack
  data_push(ctx, literal);

  debug("LIT pushed literal: %d", literal);
}

// SM/REM ( d1 n1 -- n2 n3 )  Symmetric division primitive (rounds toward zero)
// d1 is double-cell dividend, n1 is single-cell divisor
// n2 is remainder (sign of dividend), n3 is quotient (truncated)
void f_sm_rem(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Pop divisor (single cell)
  cell_t divisor = data_pop(ctx);
  require(divisor != 0);  // Division by zero check

  // Pop dividend (double cell: high cell first, then low cell)
  cell_t dividend_hi = data_pop(ctx);
  cell_t dividend_lo = data_pop(ctx);

  // Convert to 64-bit signed value
  int64_t dividend = ((int64_t)dividend_hi << 32) | (uint32_t)dividend_lo;
  int64_t div = (int64_t)divisor;

  // Perform symmetric division (truncate toward zero)
  int64_t quotient = dividend / div;   // C division truncates toward zero
  int64_t remainder = dividend % div;  // C remainder has sign of dividend

  // Ensure results fit in 32-bit cells
  require(quotient >= INT32_MIN && quotient <= INT32_MAX);
  require(remainder >= INT32_MIN && remainder <= INT32_MAX);

  // Push remainder first, then quotient
  data_push(ctx, (cell_t)remainder);
  data_push(ctx, (cell_t)quotient);
}

// FM/MOD ( d1 n1 -- n2 n3 )  Floored division primitive (rounds toward negative
// infinity) d1 is double-cell dividend, n1 is single-cell divisor n2 is
// remainder (sign of divisor), n3 is quotient (floored)
void f_fm_mod(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Pop divisor (single cell)
  cell_t divisor = data_pop(ctx);
  require(divisor != 0);  // Division by zero check

  // Pop dividend (double cell: high cell first, then low cell)
  cell_t dividend_hi = data_pop(ctx);
  cell_t dividend_lo = data_pop(ctx);

  // Convert to 64-bit signed value
  int64_t dividend = ((int64_t)dividend_hi << 32) | (uint32_t)dividend_lo;
  int64_t div = (int64_t)divisor;

  // Perform floored division
  int64_t quotient = dividend / div;
  int64_t remainder = dividend % div;

  // Adjust for floored division if remainder and divisor have different signs
  // and remainder is non-zero (this converts from symmetric to floored)
  if (remainder != 0 && ((remainder > 0) != (div > 0))) {
    quotient -= 1;
    remainder += div;
  }

  // Ensure results fit in 32-bit cells
  require(quotient >= INT32_MIN && quotient <= INT32_MAX);
  require(remainder >= INT32_MIN && remainder <= INT32_MAX);

  // Push remainder first, then quotient
  data_push(ctx, (cell_t)remainder);
  data_push(ctx, (cell_t)quotient);
}

// Bitwise logical operations - operate on the data stack

// AND ( x1 x2 -- x3 )  Bitwise logical AND of x1 with x2
void f_and(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);
  cell_t x3 = x1 & x2;  // Bitwise AND operator in C
  data_push(ctx, x3);
}

// OR ( x1 x2 -- x3 )  Bitwise inclusive-or of x1 with x2
void f_or(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);
  cell_t x3 = x1 | x2;  // Bitwise OR operator in C
  data_push(ctx, x3);
}

// XOR ( x1 x2 -- x3 )  Bitwise exclusive-or of x1 with x2
void f_xor(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x2 = data_pop(ctx);
  cell_t x1 = data_pop(ctx);
  cell_t x3 = x1 ^ x2;  // Bitwise XOR operator in C
  data_push(ctx, x3);
}

// INVERT ( x1 -- x2 )  Bitwise logical inversion of x1
void f_invert(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x1 = data_pop(ctx);
  cell_t x2 = ~x1;  // Bitwise NOT operator in C
  data_push(ctx, x2);
}

// I/O primitives - operate on the data stack and provide character I/O

// EMIT ( char -- )  Output character to the user output device
void f_emit(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t char_value = data_pop(ctx);

  // Extract character from cell (only low 8 bits)
  char c = (char)(char_value & 0xFF);

  // Output character directly to stdout
  putchar(c);
  fflush(stdout);  // Ensure immediate output like f_dot
}

// KEY ( -- char )  Input character from the user input device
void f_key(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Read one character from stdin
  int c = getchar();

  // Handle EOF or error conditions
  if (c == EOF) {
    c = 0;  // Push null character on EOF
  }

  // Push character value onto stack (extend to cell size)
  data_push(ctx, (cell_t)(c & 0xFF));
}

// TYPE ( c-addr u -- )  Output u characters from string at c-addr
void f_type(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t u = data_pop(ctx);                           // Character count
  forth_addr_t c_addr = (forth_addr_t)data_pop(ctx);  // String address

  // Bounds check the character count
  if (u < 0) {
    return;  // Ignore negative count per ANS Forth practice
  }

  // Output each character
  for (cell_t i = 0; i < u; i++) {
    // Bounds check the address
    if (c_addr + i >= FORTH_MEMORY_SIZE) {
      break;  // Stop at memory boundary
    }

    char c = (char)forth_c_fetch(c_addr + i);
    putchar(c);
  }

  fflush(stdout);  // Ensure immediate output
}

// Return stack operations - essential for colon definitions and mixed-precision
// arithmetic

// >R ( x -- ) ( R: -- x )  Transfer x from data stack to return stack
void f_to_r(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x = data_pop(ctx);
  return_push(ctx, x);
}

// R> ( -- x ) ( R: x -- )  Transfer x from return stack to data stack
void f_r_from(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x = return_pop(ctx);
  data_push(ctx, x);
}

// R@ ( -- x ) ( R: x -- x )  Copy top of return stack to data stack
void f_r_fetch(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Make sure there's something to peek at
  require(return_depth(ctx) > 0);

  // Peek at top of return stack without removing it
  cell_t x = ctx->return_stack[ctx->return_stack_ptr - 1];
  data_push(ctx, x);
}

// M* ( n1 n2 -- d )  Multiply n1 by n2 giving signed double-cell product d
void f_m_star(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);

  // Use 64-bit arithmetic to handle the full range without overflow
  int64_t product = (int64_t)n1 * (int64_t)n2;

  // Push as double-cell: low cell first, then high cell
  // This follows ANS Forth convention for double-cell numbers
  data_push(ctx, (cell_t)(product & 0xFFFFFFFF));          // low 32 bits
  data_push(ctx, (cell_t)((product >> 32) & 0xFFFFFFFF));  // high 32 bits
}

// IMMEDIATE ( -- ) Mark the most recently defined word as immediate
void f_immediate(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (dictionary_head == NULL) error(ctx, "No word to make immediate");

  // Set immediate flag on most recently defined word
  dictionary_head->flags |= WORD_FLAG_IMMEDIATE;

  debug("Made word '%s' immediate", dictionary_head->name);
}

// ROLL ( xu xu-1 ... x1 x0 u -- xu-1 ... x1 x0 xu )
// Remove u. Rotate u+1 items on top of stack. An ambiguous condition
// exists if there are less than u+2 items on the stack before ROLL.
void f_roll(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t u = data_pop(ctx);

  if (u == 0) {
    // ROLL with u=0 is a no-op
    return;
  }

  if (data_depth(ctx) < u + 1) error(ctx, "ROLL stack underflow");

  // Get the item that's u positions down
  cell_t xu = ctx->data_stack[ctx->data_stack_ptr - 1 - u];

  // Shift all items above it down by one position
  for (int i = ctx->data_stack_ptr - 1 - u; i < ctx->data_stack_ptr - 1; i++) {
    ctx->data_stack[i] = ctx->data_stack[i + 1];
  }

  // Put xu on top
  ctx->data_stack[ctx->data_stack_ptr - 1] = xu;

  debug("ROLL %d executed", u);
}

void f_dot_quote_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (ctx->ip == 0) error(ctx, "'.(' called outside colon definition");

  // Read string length from parameter field
  byte_t length = (byte_t)forth_fetch(ctx->ip);
  ctx->ip += sizeof(cell_t);

  debug("(. runtime: reading string length %d", length);

  // Display each character
  for (cell_t i = 0; i < length; i++) {
    putchar(forth_c_fetch(ctx->ip + i));
  }

  fflush(stdout);
  ctx->ip = align_up(ctx->ip + length, sizeof(cell_t));

  debug("(. runtime: displayed string, IP now at %u", ctx->ip);
}

// Runtime word for ABORT" - reads inline string data and conditionally aborts
// Compiled sequence: [(ABORT")]  [length] [char1] [char2] ... [charN]
void f_abort_quote_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t flag = data_pop(ctx);

  if (ctx->ip == 0) error(ctx, "(ABORT called outside colon definition");

  // Read string length from parameter field
  byte_t length = (byte_t)forth_fetch(ctx->ip);
  ctx->ip += sizeof(cell_t);

  cell_t start = ctx->ip;

  ctx->ip = align_up(ctx->ip + length, sizeof(cell_t));

  debug("(ABORT runtime: flag=%d, string length=%d", flag, length);

  if (flag != 0) {
    // Display the string
    for (cell_t i = 0; i < length; i++) {
      putchar(forth_c_fetch(start + i));
    }

    fflush(stdout);
    f_abort(ctx, self);
  } else {
    debug("(ABORT runtime: skipped string, IP now at %u", ctx->ip);
  }
}

// Improved ." implementation - compiles inline string data
void f_dot_quote(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Parse the string
  char string_buffer[256];
  int length = parse_string('"', string_buffer, sizeof(string_buffer));

  if (*state_ptr == 0) {
    // Interpretation mode - display immediately
    debug(".\" interpretation: displaying '%s'", string_buffer);
    printf("%s", string_buffer);
    fflush(stdout);
  } else {
    // Compilation mode - compile inline string data
    debug(".\" compilation: compiling inline string \"%s\" (length %d)",
          string_buffer, length);

    word_t* runtime_word = find_word(ctx, "(.\"");

    compile_token(ptr_to_addr(runtime_word));
    compile_token((forth_addr_t)length);

    for (int i = 0; i < length; i++) {
      forth_c_store(here + i, string_buffer[i]);
    }

    here += length;
    forth_align();  // Align for next token

    debug(".\" compilation: compiled %d bytes + alignment",
          length + sizeof(cell_t));
  }
}

// Improved ABORT" implementation - compiles inline string data
void f_abort_quote(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Parse the string
  char string_buffer[256];
  int length = parse_string('"', string_buffer, sizeof(string_buffer));

  if (*state_ptr == 0) {
    // Interpretation mode - check flag and abort immediately
    if (data_depth(ctx) < 1) error(ctx, "ABORT\" requires a flag on the stack");

    cell_t flag = data_pop(ctx);
    if (flag != 0) {
      printf("%s", string_buffer);
      f_abort(ctx, self);
    }
  } else {
    // Compilation mode - compile inline string data
    debug("ABORT\" compilation: compiling inline string \"%s\"", string_buffer);

    // 1. Compile the runtime word
    word_t* runtime_word = find_word(ctx, "(ABORT\"");

    compile_token(ptr_to_addr(runtime_word));

    // 2. Compile the string length
    compile_token((forth_addr_t)length);

    // 3. Compile each character
    for (int i = 0; i < length; i++) {
      forth_c_store(here + i, string_buffer[i]);
    }

    here += length;
    forth_align();  // Align for next token
  }
}

void f_create(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  defining_word(ctx, f_param_field);
}

void f_variable(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  defining_word(ctx, f_address)->param_field = 0;  // initialize variable to 0
}

// ['] ( "name" -- ) Compilation: ( -- ) Runtime: ( -- xt )
// Parse name, find it, compile its execution token as literal
void f_bracket_tick(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  char name_buffer[32];
  char* name = parse_name(name_buffer, sizeof(name_buffer));
  if (!name) error(ctx, "Missing name after [']");

  word_t* word = find_word(ctx, name);
  if (!word) error(ctx, "Word not found in [']");

  // Compile the word's address as a literal
  compile_literal(ptr_to_addr(word));

  debug("['] compiled literal for %s: %u", name, ptr_to_addr(word));
}

// 0BRANCH ( x -- ) - conditional branch
// If x is zero, branch to address stored at ctx->ip
// Always advances ctx->ip past the address
void f_0branch(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t x = data_pop(ctx);
  forth_addr_t target = forth_fetch(ctx->ip);
  ctx->ip += sizeof(cell_t);

  if (x == 0) {
    ctx->ip = target;  // Branch taken
  }
  // If x != 0, continue (branch not taken)
}

// BRANCH ( -- ) - unconditional branch
void f_branch(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t target = forth_fetch(ctx->ip);
  ctx->ip = target;  // Always branch
}

// U< ( u1 u2 -- flag )  Unsigned less than comparison
void f_u_less(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  cell_t n2 = data_pop(ctx);
  cell_t n1 = data_pop(ctx);

  // Cast to unsigned for comparison
  uint32_t u1 = (uint32_t)n1;
  uint32_t u2 = (uint32_t)n2;

  data_push(ctx, u1 < u2 ? -1 : 0);
}

// ' ( "<spaces>name" -- xt )  Parse name and return execution token
void f_tick(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  char name_buffer[32];
  char* name = parse_name(name_buffer, sizeof(name_buffer));

  if (!name) {
    error(ctx, "' expects a name");
  }

  debug("' looking for word: %s", name);

  word_t* word = find_word(ctx, name);

  // Convert word pointer to execution token (forth address)
  forth_addr_t xt = ptr_to_addr(word);
  data_push(ctx, (cell_t)xt);

  debug("' found word %s at address %u", name, xt);
}

// EXECUTE ( i*x xt -- j*x )  Execute the word whose execution token is xt
void f_execute(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t xt = (forth_addr_t)data_pop(ctx);

  debug("EXECUTE: executing token at address %u", xt);

  // Convert execution token back to word pointer
  word_t* word = addr_to_ptr(ctx, xt);

  debug("EXECUTE: found word %s", word->name);

  // Execute the word
  execute_word(ctx, word);
}

// FIND ( c-addr -- c-addr 0 | xt 1 | xt -1 )  Find word in dictionary
void f_find(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  forth_addr_t c_addr = (forth_addr_t)data_pop(ctx);

  // Get the counted string: first byte is length, followed by characters
  byte_t length = forth_c_fetch(c_addr);

  // Convert counted string to null-terminated C string
  char name_buffer[32];
  if (length >= sizeof(name_buffer)) {
    // String too long, not found
    data_push(ctx, (cell_t)c_addr);
    data_push(ctx, 0);
    return;
  }

  for (int i = 0; i < length; i++) {
    name_buffer[i] = forth_c_fetch(c_addr + 1 + i);
  }
  name_buffer[length] = '\0';

  debug("FIND looking for word: %s", name_buffer);

  // Search for the word
  word_t* word = search_word(name_buffer);

  if (!word) {
    // Not found: return c-addr 0
    data_push(ctx, (cell_t)c_addr);
    data_push(ctx, 0);
    debug("FIND: word not found");
  } else {
    // Found: return xt and flag
    forth_addr_t xt = ptr_to_addr(word);
    data_push(ctx, (cell_t)xt);

    // Check if immediate
    if (word->flags & WORD_FLAG_IMMEDIATE) {
      data_push(ctx, -1);  // Immediate word
      debug("FIND: found immediate word %s at %u", name_buffer, xt);
    } else {
      data_push(ctx, 1);  // Normal word
      debug("FIND: found normal word %s at %u", name_buffer, xt);
    }
  }
}

void f_unused(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;
  cell_t unused_bytes = FORTH_MEMORY_SIZE - here;
  data_push(ctx, unused_bytes);
}

// DO runtime: ( limit start -- ) ( R: -- loop-sys )
// Sets up loop parameters on return stack
void f_do_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (data_depth(ctx) < 2) {
    error(ctx, "DO requires 2 items on stack");
  }

  cell_t start = data_pop(ctx);  // Loop index (start value)
  cell_t limit = data_pop(ctx);  // Loop limit

  // Push loop parameters onto return stack: limit first, then index
  return_push(ctx, limit);
  return_push(ctx, start);

  debug("DO: limit=%d, start=%d", limit, start);
}

// LOOP runtime: ( -- ) ( R: loop-sys1 -- | loop-sys2 )
// Increment index by 1, test for loop termination
void f_loop_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (return_depth(ctx) < 2) {
    error(ctx, "LOOP: missing loop parameters on return stack");
  }

  // Get loop parameters from return stack
  cell_t index = return_pop(ctx);
  cell_t limit = return_pop(ctx);

  // Increment index
  index++;

  debug("LOOP: index=%d, limit=%d", index, limit);

  // Check for loop termination
  if (index == limit) {
    // Loop finished - don't restore parameters, continue after loop
    ctx->ip += sizeof(cell_t);  // Skip over the branch target address
    debug("LOOP: finished");
    return;
  }

  // Continue loop - restore parameters and branch back
  return_push(ctx, limit);
  return_push(ctx, index);

  // The backward branch address follows this instruction
  forth_addr_t branch_target = forth_fetch(ctx->ip);
  ctx->ip = branch_target;
  debug("LOOP: continue to %d", branch_target);
}

// +LOOP runtime: ( n -- ) ( R: loop-sys1 -- | loop-sys2 )
// Increment index by n, test for loop termination with boundary crossing
void f_plus_loop_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (data_depth(ctx) < 1) {
    error(ctx, "+LOOP requires 1 item on stack");
  }

  if (return_depth(ctx) < 2) {
    error(ctx, "+LOOP: missing loop parameters on return stack");
  }

  cell_t increment = data_pop(ctx);

  // Get loop parameters from return stack
  cell_t index = return_pop(ctx);
  cell_t limit = return_pop(ctx);

  cell_t old_index = index;
  index += increment;

  debug("+LOOP: old_index=%d, increment=%d, new_index=%d, limit=%d", old_index,
        increment, index, limit);

  // Check for boundary crossing (ANS Forth standard)
  // Loop terminates when index crosses the boundary between limit-1 and limit
  bool crossed = false;

  if (increment > 0) {
    // Positive increment: terminate if we crossed from below limit to >= limit
    crossed = (old_index < limit && index >= limit);
  } else if (increment < 0) {
    // Negative increment: terminate if we crossed from >= limit to < limit
    crossed = (old_index >= limit && index < limit);
  }
  // If increment is 0, never terminate (infinite loop)

  if (crossed) {
    // Loop finished - don't restore parameters
    ctx->ip += sizeof(cell_t);  // Skip over the branch target address
    debug("+LOOP: boundary crossed, finished");
    return;
  }

  // Continue loop - restore parameters and branch back
  return_push(ctx, limit);
  return_push(ctx, index);

  // The backward branch address follows this instruction
  forth_addr_t branch_target = forth_fetch(ctx->ip);
  ctx->ip = branch_target;
  debug("+LOOP: continue to %d", branch_target);
}

// I: ( -- n ) ( R: loop-sys -- loop-sys )
// Return current loop index
void f_i(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (return_depth(ctx) < 2) {
    error(ctx, "I: no loop parameters on return stack");
  }

  // Index is on top of return stack, limit is below it
  cell_t index = return_stack_peek(ctx, 0);  // Top of return stack
  data_push(ctx, index);

  debug("I: index=%d", index);
}

// J: ( -- n ) ( R: loop-sys1 loop-sys2 -- loop-sys1 loop-sys2 )
// Return outer loop index (for nested loops)
void f_j(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (return_depth(ctx) < 4) {
    error(ctx, "J: no outer loop parameters on return stack");
  }

  // Return stack layout: [outer_limit] [outer_index] [inner_limit]
  // [inner_index] J needs the outer_index, which is at position 2 from top
  // (0-indexed)
  cell_t outer_index = return_stack_peek(ctx, 2);
  data_push(ctx, outer_index);

  debug("J: outer_index=%d", outer_index);
}

// LEAVE runtime: ( -- ) ( R: loop-sys -- )
// Remove loop parameters and branch to after loop
void f_leave_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (return_depth(ctx) < 2) {
    error(ctx, "LEAVE: no loop parameters on return stack");
  }

  // Remove loop parameters
  return_pop(ctx);  // index
  return_pop(ctx);  // limit

  // The branch target address follows this instruction
  forth_addr_t branch_target = forth_fetch(ctx->ip);
  ctx->ip = branch_target;

  debug("LEAVE: branch to %d", branch_target);
}

// UNLOOP: ( -- ) ( R: loop-sys -- )
// Remove loop parameters without branching
void f_unloop(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (return_depth(ctx) < 2) {
    error(ctx, "UNLOOP: no loop parameters on return stack");
  }

  // Remove loop parameters
  return_pop(ctx);  // index
  return_pop(ctx);  // limit

  debug("UNLOOP: removed loop parameters");
}

// DO: ( C: -- do-sys )
// Compilation: Push loop frame, compile DO runtime
void f_do(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (*state_ptr == 0) {
    error(ctx, "DO can only be used in compilation mode");
  }

  // Compile the DO runtime primitive
  word_t* do_runtime = find_word(ctx, "(DO)");
  if (!do_runtime) {
    error(ctx, "DO: (DO) runtime primitive not found");
  }
  compile_word(ctx, do_runtime);

  // Push loop frame with current address as loop start
  forth_addr_t loop_start = here;
  push_loop_frame(ctx, loop_start);

  debug("DO: compiled, loop starts at %d", loop_start);
}

// LOOP: ( C: do-sys -- )
// Compilation: Resolve LEAVEs, compile LOOP runtime and backward branch
void f_loop(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (*state_ptr == 0) {
    error(ctx, "LOOP can only be used in compilation mode");
  }

  // Get and pop the loop frame
  loop_frame_t frame = pop_loop_frame(ctx);

  // Compile the LOOP runtime primitive
  word_t* loop_runtime = find_word(ctx, "(LOOP)");
  if (!loop_runtime) {
    error(ctx, "LOOP: (LOOP) runtime primitive not found");
  }
  compile_word(ctx, loop_runtime);

  // Compile backward branch target (loop start address)
  compile_cell(frame.loop_start_addr);

  // Resolve all LEAVE addresses to point here (after the loop)
  forth_addr_t after_loop = here;
  for (int i = 0; i < frame.leave_count; i++) {
    forth_addr_t addr = frame.leave_addrs[i];
    cell_t* cell_ptr = addr_to_ptr(ctx, addr);
    *cell_ptr = after_loop;
    debug("LOOP: resolved LEAVE at %d to %d", addr, after_loop);
  }

  debug("LOOP: compiled, %d LEAVEs resolved to %d", frame.leave_count,
        after_loop);
}

// +LOOP: ( C: do-sys -- )
// Compilation: Resolve LEAVEs, compile +LOOP runtime and backward branch
void f_plus_loop(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (*state_ptr == 0) {
    error(ctx, "+LOOP can only be used in compilation mode");
  }

  // Get and pop the loop frame
  loop_frame_t frame = pop_loop_frame(ctx);

  // Compile the +LOOP runtime primitive
  word_t* plus_loop_runtime = find_word(ctx, "(+LOOP)");
  if (!plus_loop_runtime) {
    error(ctx, "+LOOP: (+LOOP) runtime primitive not found");
  }
  compile_word(ctx, plus_loop_runtime);

  // Compile backward branch target (loop start address)
  compile_cell(frame.loop_start_addr);

  // Resolve all LEAVE addresses to point here (after the loop)
  forth_addr_t after_loop = here;
  for (int i = 0; i < frame.leave_count; i++) {
    forth_addr_t addr = frame.leave_addrs[i];
    cell_t* cell_ptr = addr_to_ptr(ctx, addr);
    *cell_ptr = after_loop;
    debug("+LOOP: resolved LEAVE at %d to %d", addr, after_loop);
  }

  debug("+LOOP: compiled, %d LEAVEs resolved to %d", frame.leave_count,
        after_loop);
}

// LEAVE: ( C: -- )
// Compilation: Compile LEAVE runtime and add branch address for later
// resolution
void f_leave(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;  // Unused parameter

  if (*state_ptr == 0) {
    error(ctx, "LEAVE can only be used in compilation mode");
  }

  // Compile the LEAVE runtime primitive
  word_t* leave_runtime = find_word(ctx, "(LEAVE)");
  if (!leave_runtime) {
    error(ctx, "LEAVE: (LEAVE) runtime primitive not found");
  }
  compile_word(ctx, leave_runtime);

  // Compile placeholder for branch target (will be resolved by LOOP/+LOOP)
  forth_addr_t placeholder_addr = here;
  compile_cell(0);  // Placeholder address

  // Add this address to the current loop frame for later resolution
  add_leave_addr(ctx, placeholder_addr);

  debug("LEAVE: compiled with placeholder at %d", placeholder_addr);
}

// WORD ( char "<chars>cchar<chars>" -- c-addr )
// Skip leading delimiters, parse until next delimiter, store as counted string
// in PAD
void f_word(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Get delimiter character from stack
  cell_t delimiter = data_pop(ctx);
  char delim_char = (char)(delimiter & 0xFF);

  // Get current input state
  cell_t current_to_in = forth_fetch(to_in_addr);
  cell_t current_length = forth_fetch(input_length_addr);

  // Skip leading delimiters
  while (current_to_in < current_length) {
    char ch = (char)forth_c_fetch(input_buffer_addr + current_to_in);
    if (ch != delim_char) {
      break;  // Found non-delimiter
    }
    current_to_in++;
  }

  // Parse until next delimiter or end of input
  cell_t start_pos = current_to_in;
  while (current_to_in < current_length) {
    char ch = (char)forth_c_fetch(input_buffer_addr + current_to_in);
    if (ch == delim_char) {
      break;  // Found delimiter
    }
    current_to_in++;
  }

  // Calculate length of parsed string
  cell_t length = current_to_in - start_pos;

  // Ensure length fits in a byte (ANS Forth requirement)
  if (length > 255) {
    length = 255;
  }

  // Update >IN to position after delimiter (if found)
  if (current_to_in < current_length) {
    current_to_in++;  // Skip the delimiter
  }
  forth_store(to_in_addr, current_to_in);

  // Get PAD address by executing PAD word
  word_t* pad_word = search_word("PAD");
  if (!pad_word) {
    error(ctx, "WORD: PAD not found");
  }
  pad_word->cfunc(ctx, pad_word);  // Execute PAD to get address
  forth_addr_t pad_addr = (forth_addr_t)data_pop(ctx);

  // Store counted string in PAD
  forth_c_store(pad_addr, (byte_t)length);  // Store length byte

  // Copy characters from input buffer to PAD
  for (cell_t i = 0; i < length; i++) {
    char ch = (char)forth_c_fetch(input_buffer_addr + start_pos + i);
    forth_c_store(pad_addr + 1 + i, (byte_t)ch);
  }

  // Return PAD address
  data_push(ctx, (cell_t)pad_addr);

  debug("WORD: delimiter='%c', parsed \"%.*s\" (length %d)", delim_char,
        (int)length, (char*)&forth_memory[input_buffer_addr + start_pos],
        (int)length);
}

// ACCEPT ( c-addr +n1 -- +n2 )
// Read up to +n1 characters into buffer at c-addr, return actual count
void f_accept(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Get parameters from stack
  cell_t max_chars = data_pop(ctx);
  forth_addr_t buffer_addr = (forth_addr_t)data_pop(ctx);

  // Validate parameters
  if (max_chars < 0) {
    data_push(ctx, 0);
    return;
  }

  cell_t count = 0;

  debug("ACCEPT: reading up to %d characters into buffer at %u", max_chars,
        buffer_addr);

  while (count < max_chars) {
    // Read one character using KEY
    word_t* key_word = search_word("KEY");
    if (!key_word) {
      error(ctx, "ACCEPT: KEY not found");
    }
    key_word->cfunc(ctx, key_word);  // Execute KEY
    cell_t char_value = data_pop(ctx);

    char ch = (char)(char_value & 0xFF);

    // Check for line terminators
    if (ch == '\r' || ch == '\n') {
      // End of line - stop reading
      break;
    }

    // Check for backspace/delete
    if (ch == '\b' || ch == '\x7F') {
      if (count > 0) {
        count--;
        // Echo backspace sequence: backspace, space, backspace
        putchar('\b');
        putchar(' ');
        putchar('\b');
        fflush(stdout);
      }
      continue;
    }

    // Store character in buffer
    forth_c_store(buffer_addr + count, (byte_t)ch);
    count++;

    // Echo character to output (for interactive use)
    putchar(ch);
    fflush(stdout);
  }

  // Return actual count
  data_push(ctx, count);

  debug("ACCEPT: read %d characters", count);
}

// S" runtime implementation - pushes address and length of compiled string
void f_s_quote_runtime(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (ctx->ip == 0) error(ctx, "(S\") called outside colon definition");

  // Read the string length from the instruction stream
  cell_t length = forth_fetch(ctx->ip);
  ctx->ip += sizeof(cell_t);

  // Push the string address and length onto the data stack
  data_push(ctx, (cell_t)ctx->ip);  // c-addr
  data_push(ctx, length);           // u

  // Advance IP past the string data
  ctx->ip += length;
  ctx->ip = (ctx->ip + sizeof(cell_t) - 1) & ~(sizeof(cell_t) - 1);  // Align

  debug("(S\") pushed addr=%d length=%d", ctx->ip - length - sizeof(cell_t),
        length);
}

// S" implementation - compiles inline string data
void f_s_quote(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  // Parse the string
  char string_buffer[256];
  int length = parse_string('"', string_buffer, sizeof(string_buffer));

  if (*state_ptr == 0) {
    // Interpretation mode - store string temporarily and push address/length
    debug("S\" interpretation: storing '%s' temporarily", string_buffer);
    // Store string at PAD (temporary area)
    forth_addr_t pad_addr = here - FORTH_PAD_SIZE;  // PAD is before HERE
    for (int i = 0; i < length; i++) {
      forth_c_store(pad_addr + i, string_buffer[i]);
    }

    // Push address and length
    data_push(ctx, (cell_t)pad_addr);  // c-addr
    data_push(ctx, length);            // u

  } else {
    // Compilation mode - compile inline string data
    debug("S\" compilation: compiling inline string \"%s\" (length %d)",
          string_buffer, length);

    // 1. Compile the runtime word
    word_t* runtime_word = find_word(ctx, "(S\")");
    compile_word(ctx, runtime_word);

    // 2. Compile the string length
    compile_cell((cell_t)length);

    // 3. Compile the string data
    for (int i = 0; i < length; i++) {
      forth_c_store(here + i, string_buffer[i]);
    }

    here += length;
    forth_align();  // Align for next cell

    debug("S\" compilation: compiled %d bytes + alignment",
          length + sizeof(cell_t));
  }
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
  create_primitive_word("ABORT", f_abort);
  create_primitive_word("BYE", f_bye);
  create_primitive_word(".", f_dot);
  create_primitive_word("!", f_store);
  create_primitive_word("@", f_fetch);
  create_primitive_word("C!", f_c_store);
  create_primitive_word("C@", f_c_fetch);
  create_primitive_word("=", f_equals);
  create_primitive_word("<", f_less_than);
  create_primitive_word("0=", f_zero_equals);
  create_primitive_word("SWAP", f_swap);
  create_primitive_word("ROT", f_rot);
  create_primitive_word("PICK", f_pick);
  create_primitive_word("ROLL", f_roll);
  create_primitive_word("HERE", f_here);
  create_primitive_word("ALLOT", f_allot);
  create_primitive_word(",", f_comma);
  create_primitive_word("LIT", f_lit);
  create_primitive_word("SM/REM", f_sm_rem);
  create_primitive_word("FM/MOD", f_fm_mod);
  create_primitive_word("AND", f_and);
  create_primitive_word("OR", f_or);
  create_primitive_word("XOR", f_xor);
  create_primitive_word("INVERT", f_invert);
  create_primitive_word("EMIT", f_emit);
  create_primitive_word("KEY", f_key);
  create_primitive_word("TYPE", f_type);
  create_primitive_word(">R", f_to_r);
  create_primitive_word("R>", f_r_from);
  create_primitive_word("R@", f_r_fetch);
  create_primitive_word("M*", f_m_star);
  create_primitive_word("U<", f_u_less);

  // Create STATE variable (0 = interpret, -1 = compile)
  state_ptr = create_variable_word("STATE", 0);

  // Create BASE variable (default to decimal)
  base_ptr = create_variable_word("BASE", 10);

  create_primitive_word(":", f_colon);
  create_immediate_primitive_word(";", f_semicolon);
  create_primitive_word("EXIT", f_exit);
  create_primitive_word("IMMEDIATE", f_immediate);

  // Create helper words first (these are implementation details)
  create_primitive_word("(S\")", f_s_quote_runtime);
  create_primitive_word("(.\"", f_dot_quote_runtime);
  create_primitive_word("(ABORT\"", f_abort_quote_runtime);

  // Create the user-visible immediate words
  create_immediate_primitive_word("S\"", f_s_quote);
  create_immediate_primitive_word(".\"", f_dot_quote);
  create_immediate_primitive_word("ABORT\"", f_abort_quote);

  create_area_word("PAD");
  forth_allot(FORTH_PAD_SIZE);

  create_primitive_word("CREATE", f_create);
  create_primitive_word("VARIABLE", f_variable);

  create_primitive_word("0BRANCH", f_0branch);
  create_primitive_word("BRANCH", f_branch);
  create_immediate_primitive_word("[']", f_bracket_tick);
  create_primitive_word("'", f_tick);
  create_primitive_word("EXECUTE", f_execute);
  create_primitive_word("FIND", f_find);
  create_primitive_word("UNUSED", f_unused);

  // Runtime primitives (not immediate)
  create_primitive_word("(DO)", f_do_runtime);
  create_primitive_word("(LOOP)", f_loop_runtime);
  create_primitive_word("(+LOOP)", f_plus_loop_runtime);
  create_primitive_word("(LEAVE)", f_leave_runtime);
  create_primitive_word("I", f_i);
  create_primitive_word("J", f_j);
  create_primitive_word("UNLOOP", f_unloop);

  // Compilation words (immediate)
  create_immediate_primitive_word("DO", f_do);
  create_immediate_primitive_word("LOOP", f_loop);
  create_immediate_primitive_word("+LOOP", f_plus_loop);
  create_immediate_primitive_word("LEAVE", f_leave);

  create_primitive_word("WORD", f_word);
  create_primitive_word("ACCEPT", f_accept);
}

// Built-in Forth definitions (created after primitives are available)
static const char* builtin_definitions[] = {
    ": IF    ['] 0BRANCH ,  HERE  0 , ; IMMEDIATE",
    ": THEN  HERE  SWAP  ! ; IMMEDIATE",
    ": ELSE  ['] BRANCH ,  HERE  0 ,  SWAP  HERE  SWAP  ! ; IMMEDIATE",

    // Stack manipulation words
    ": DUP 0 PICK ;", ": OVER 1 PICK ;", ": 2DUP OVER OVER ;",
    ": NIP SWAP DROP ;", ": TUCK SWAP OVER ;", ": 2DROP DROP DROP ;",
    ": 2SWAP ROT >R ROT R> ;", ": 2OVER 3 PICK 3 PICK ;",
    ": ?DUP DUP IF DUP THEN ;",

    ": TRUE -1 ;", ": FALSE 0 ;",

    ": NEGATE 0 SWAP - ;", ": 1+ 1 + ;", ": 1- 1 - ;",

    ": 0< 0 < ;", ": 0> 0 SWAP < ;", ": > SWAP < ;", ": NOT 0= ;",
    ": <> = NOT ;", ": 0<> 0 <> ;", ": <= > NOT ;", ": >= < NOT ;",
    ": U> SWAP U< ;",  // Unsigned greater than
    ": U<= U> NOT ;",  // Unsigned less than or equal
    ": U>= U< NOT ;",  // Unsigned greater than or equal
    ": 2* DUP + ;", ": 2/ 2 / ;",

    ": MOD SM/REM DROP ;",
    ": /MOD DUP >R 0 SWAP SM/REM R> 0< IF SWAP NEGATE SWAP THEN ;",
    ": */ >R M* R> FM/MOD SWAP DROP ;", ": */MOD >R M* R> FM/MOD ;",

    ": CELL+ 4 + ;", ": CELLS 4 * ;", ": CHAR+ 1+ ;",
    ": CHARS ;",  // No-op
    ": +! TUCK @ + SWAP ! ;", ": 2! TUCK ! CELL+ ! ;",
    ": 2@ DUP CELL+ @ SWAP @ ;",

    // State control (immediate words)
    ": [ 0 STATE ! ; IMMEDIATE",   // Enter interpretation state
    ": ] -1 STATE ! ; IMMEDIATE",  // Enter compilation state

    ": DECIMAL 10 BASE ! ;", ": HEX 16 BASE ! ;", ": BINARY 2 BASE ! ;",
    ": OCTAL 8 BASE ! ;",

    ": BL 32 ;", ": CR 10 EMIT ;",

    // ?DUP ( x -- 0 | x x ) - Duplicate if non-zero
    ": ?DUP DUP IF DUP THEN ;",

    // ABS ( n -- u ) - Absolute value
    ": ABS DUP 0< IF NEGATE THEN ;",

    // MIN ( n1 n2 -- n3 ) - Return the lesser value
    ": MIN 2DUP > IF SWAP THEN DROP ;",

    // MAX ( n1 n2 -- n3 ) - Return the greater value
    ": MAX 2DUP < IF SWAP THEN DROP ;",

    // WITHIN ( n1|u1 n2|u2 n3|u3 -- flag ) - Core Extension but very useful
    // Returns true if n2 <= n1 < n3 (when n2 < n3) or if n2 <= n1 OR n1 < n3
    // (when n2 >= n3)
    ": WITHIN OVER - >R - R> U< ;",

    // SIGNUM ( n -- -1|0|1 ) - Not required but helpful
    ": SIGNUM DUP 0< IF DROP -1 ELSE 0> IF 1 ELSE 0 THEN THEN ;",

    // BOUNDS ( addr1 u -- addr2 addr1 ) - Not required but useful for loops
    ": BOUNDS OVER + SWAP ;",

    ": BEGIN  HERE ; IMMEDIATE", ": AGAIN  ['] BRANCH , , ; IMMEDIATE",
    ": UNTIL  ['] 0BRANCH , , ; IMMEDIATE",
    ": WHILE  ['] 0BRANCH , HERE 0 , SWAP ; IMMEDIATE",
    ": REPEAT ['] BRANCH , , HERE SWAP ! ; IMMEDIATE",

    ": SPACE BL EMIT ;", ": SPACES BEGIN DUP WHILE SPACE 1- REPEAT DROP ;",

    ": ALIGN HERE 3 + 3 INVERT AND HERE - ALLOT ;",
    ": ALIGNED 3 + 3 INVERT AND ;",

    NULL  // End marker
};

// Create all built-in colon definitions
void create_builtin_definitions(void) {
  debug("Creating built-in colon definitions...");

  for (int i = 0; builtin_definitions[i] != NULL; i++) {
    debug("  Defining: %s", builtin_definitions[i]);

    // Save current state
    cell_t saved_state = *state_ptr;

    // Interpret the definition
    interpret_text(builtin_definitions[i]);

    // Verify we're back in interpretation state
    if (*state_ptr != 0) {
      printf("Built-in definition left system in compilation state: %s",
             builtin_definitions[i]);
      abort();
    }

    // Restore state if it was somehow changed
    if (saved_state == 0 && *state_ptr != 0) {
      *state_ptr = saved_state;
    }
  }

  debug("Built-in definitions complete");
}
