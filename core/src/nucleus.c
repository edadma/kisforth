#include "forth.h"
#include "debug.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>

// Global STATE pointer for efficient access
cell_t* state_ptr = NULL;

// Global instruction pointer for colon definition execution
static forth_addr_t current_ip = 0;  // 0 means not executing

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

cell_t* create_variable_word(const char* name, cell_t initial_value) {
    // Create the word header with f_variable cfunc
    create_primitive_word(name, f_variable);

    // Now allocate space for the parameter field (one cell)
    forth_addr_t param_addr = forth_allot(sizeof(cell_t));

    // Store the initial value
    forth_store(param_addr, initial_value);

    // Return C pointer to the parameter field for efficiency
    return (cell_t*)&forth_memory[param_addr];
}

// Simple helper function
word_t* create_immediate_primitive_word(const char* name, void (*cfunc)(word_t* self)) {
    word_t* word = create_primitive_word(name, cfunc);
    word->flags |= WORD_FLAG_IMMEDIATE;
    return word;
}

// Execute a word by calling its cfunc
void execute_word(word_t* word) {
    assert(word != NULL);
    assert(word->cfunc != NULL);
    word->cfunc(word);
}

void f_variable(word_t* self) {
    // Parameter field is right after the word structure in memory
    forth_addr_t word_addr = word_to_addr(self);
    forth_addr_t param_addr = word_addr + sizeof(word_t);

    // Push the Forth address of the parameter field
    data_push(param_addr);
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

// SWAP ( x1 x2 -- x2 x1 )  Exchange the top two stack items
void f_swap(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();
    data_push(x2);
    data_push(x1);
}

// ROT ( x1 x2 x3 -- x2 x3 x1 )  Rotate third item to top
void f_rot(word_t* self) {
    (void)self;

    cell_t x3 = data_pop();
    cell_t x2 = data_pop();
    cell_t x1 = data_pop();
    data_push(x2);
    data_push(x3);
    data_push(x1);
}

// PICK ( xu ... x1 x0 u -- xu ... x1 x0 xu )  Copy u-th stack item to top
// 0 PICK is equivalent to DUP, 1 PICK is equivalent to OVER
void f_pick(word_t* self) {
    (void)self;

    cell_t u = data_pop();

    // Bounds check: u must be >= 0 and < stack depth
    assert(u >= 0);
    assert(u < data_depth());

    // Use data_peek_at to get the u-th item from top
    cell_t xu = data_peek_at(u);
    data_push(xu);
}

// HERE ( -- addr )  Return the current data space pointer
void f_here(word_t* self) {
    (void)self;

    data_push(here);
}

// ALLOT ( n -- )  Allocate n bytes of data space
void f_allot(word_t* self) {
    (void)self;

    cell_t n = data_pop();

    // Handle negative allot (deallocation) carefully
    if (n < 0) {
        // Negative allot - check bounds to prevent underflow
        assert(here >= (forth_addr_t)(-n));
        here += n;  // n is negative, so this subtracts
    } else {
        // Positive allot - normal allocation
        forth_allot(n);
    }
}

// , ( x -- )  Store cell at HERE and advance HERE by one cell
void f_comma(word_t* self) {
    (void)self;

    cell_t x = data_pop();

    // Align HERE to cell boundary before storing
    forth_align();

    // Store the value at current HERE
    forth_store(here, x);

    // Advance HERE by one cell
    here += sizeof(cell_t);
}

// Execute a colon definition using the return stack
void execute_colon(word_t* self) {
    // Parameter field contains array of tokens (word addresses)
    forth_addr_t word_addr = word_to_addr(self);
    forth_addr_t tokens_addr = word_addr + sizeof(word_t);

    debug("Executing colon definition: %s", self->name);

    // Save current instruction pointer on return stack (if executing)
    if (current_ip != 0) {
        return_push((cell_t)current_ip);
        debug("  Saved IP on return stack");
    }

    // Set new instruction pointer to start of this definition's tokens
    current_ip = tokens_addr;

    // Execute tokens until EXIT is called (which will restore IP from return stack)
    while (current_ip != 0) {
        forth_addr_t token_addr = forth_fetch(current_ip);
        current_ip += sizeof(cell_t);  // Advance to next token

        // Execute the word at token_addr
        word_t* word = addr_to_word(token_addr);
        debug("  Executing token: %s", word->name);
        execute_word(word);

        // If EXIT was called, current_ip will have been updated
    }

    debug("Colon definition execution complete");
}

// : (colon) - start colon definition
// ( C: "<spaces>name" -- colon-sys )
void f_colon(word_t* self) {
    (void)self;

    char name_buffer[32];

    // Parse the name for the new definition
    char* name = parse_name(name_buffer, sizeof(name_buffer));
    if (!name) {
        printf("ERROR: Missing name after :\n");
        return;
    }

    debug("Starting colon definition: %s", name);

    // Create word header but don't link it yet (hidden until ; is executed)
    forth_align();
    current_def_addr = here;
    forth_addr_t word_addr = forth_allot(sizeof(word_t));
    word_t* word = addr_to_word(word_addr);

    // Initialize word header
    word->link = NULL;  // Will be set by ; when definition is complete
    strncpy(word->name, name, sizeof(word->name) - 1);
    word->name[sizeof(word->name) - 1] = '\0';
    word->flags = 0;
    word->cfunc = execute_colon;

    // Enter compilation state
    *state_ptr = -1;

    debug("Colon definition header created at %u, entering compilation mode", word_addr);
}

// ; (semicolon) - end colon definition
// Compilation: ( C: colon-sys -- )
void f_semicolon(word_t* self) {
    (void)self;

    if (*state_ptr == 0) {
        printf("ERROR: ; without matching :\n");
        return;
    }

    debug("Ending colon definition, compiling EXIT");

    // Compile EXIT as the last token
    word_t* exit_word = find_word("EXIT");
    if (exit_word) {
        compile_token(word_to_addr(exit_word));
    } else {
        printf("ERROR: EXIT word not found\n");
        return;
    }

    // Link the completed word into dictionary
    word_t* word = addr_to_word(current_def_addr);
    link_word(word);

    // Exit compilation state
    *state_ptr = 0;
    current_def_addr = 0;

    debug("Colon definition complete, exiting compilation mode");
}

// EXIT - return from colon definition using return stack
// Run-time: ( -- ) ( R: nest-sys -- )
void f_exit(word_t* self) {
    (void)self;

    debug("EXIT called");

    // Check if there's a saved instruction pointer on the return stack
    if (return_depth() > 0) {
        // Restore previous instruction pointer from return stack
        current_ip = (forth_addr_t)return_pop();
        debug("  Restored IP from return stack");
    } else {
        // No saved IP - we're at the top level, end execution
        current_ip = 0;
        debug("  No saved IP - ending execution");
    }
}

// LIT implementation that reads from instruction stream
// LIT ( -- x ) Push the literal value that follows in compiled code
void f_lit(word_t* self) {
    (void)self;

    if (current_ip == 0) {
        printf("ERROR: LIT called outside colon definition\n");
        data_push(0);  // Fallback for safety
        return;
    }

    // Read the literal value from the instruction stream
    cell_t literal = forth_fetch(current_ip);
    current_ip += sizeof(cell_t);  // Advance past the literal

    // Push the literal onto the data stack
    data_push(literal);

    debug("LIT pushed literal: %d", literal);
}

// SM/REM ( d1 n1 -- n2 n3 )  Symmetric division primitive (rounds toward zero)
// d1 is double-cell dividend, n1 is single-cell divisor
// n2 is remainder (sign of dividend), n3 is quotient (truncated)
void f_sm_rem(word_t* self) {
    (void)self;

    // Pop divisor (single cell)
    cell_t divisor = data_pop();
    assert(divisor != 0);  // Division by zero check

    // Pop dividend (double cell: high cell first, then low cell)
    cell_t dividend_hi = data_pop();
    cell_t dividend_lo = data_pop();

    // Convert to 64-bit signed value
    int64_t dividend = ((int64_t)dividend_hi << 32) | (uint32_t)dividend_lo;
    int64_t div = (int64_t)divisor;

    // Perform symmetric division (truncate toward zero)
    int64_t quotient = dividend / div;  // C division truncates toward zero
    int64_t remainder = dividend % div;  // C remainder has sign of dividend

    // Ensure results fit in 32-bit cells
    assert(quotient >= INT32_MIN && quotient <= INT32_MAX);
    assert(remainder >= INT32_MIN && remainder <= INT32_MAX);

    // Push remainder first, then quotient
    data_push((cell_t)remainder);
    data_push((cell_t)quotient);
}

// FM/MOD ( d1 n1 -- n2 n3 )  Floored division primitive (rounds toward negative infinity)
// d1 is double-cell dividend, n1 is single-cell divisor
// n2 is remainder (sign of divisor), n3 is quotient (floored)
void f_fm_mod(word_t* self) {
    (void)self;

    // Pop divisor (single cell)
    cell_t divisor = data_pop();
    assert(divisor != 0);  // Division by zero check

    // Pop dividend (double cell: high cell first, then low cell)
    cell_t dividend_hi = data_pop();
    cell_t dividend_lo = data_pop();

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
    assert(quotient >= INT32_MIN && quotient <= INT32_MAX);
    assert(remainder >= INT32_MIN && remainder <= INT32_MAX);

    // Push remainder first, then quotient
    data_push((cell_t)remainder);
    data_push((cell_t)quotient);
}

// Bitwise logical operations - operate on the data stack

// AND ( x1 x2 -- x3 )  Bitwise logical AND of x1 with x2
void f_and(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();
    cell_t x3 = x1 & x2;  // Bitwise AND operator in C
    data_push(x3);
}

// OR ( x1 x2 -- x3 )  Bitwise inclusive-or of x1 with x2
void f_or(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();
    cell_t x3 = x1 | x2;  // Bitwise OR operator in C
    data_push(x3);
}

// XOR ( x1 x2 -- x3 )  Bitwise exclusive-or of x1 with x2
void f_xor(word_t* self) {
    (void)self;

    cell_t x2 = data_pop();
    cell_t x1 = data_pop();
    cell_t x3 = x1 ^ x2;  // Bitwise XOR operator in C
    data_push(x3);
}

// INVERT ( x1 -- x2 )  Bitwise logical inversion of x1
void f_invert(word_t* self) {
    (void)self;

    cell_t x1 = data_pop();
    cell_t x2 = ~x1;  // Bitwise NOT operator in C
    data_push(x2);
}

// I/O primitives - operate on the data stack and provide character I/O

// EMIT ( char -- )  Output character to the user output device
void f_emit(word_t* self) {
    (void)self;

    cell_t char_value = data_pop();

    // Extract character from cell (only low 8 bits)
    char c = (char)(char_value & 0xFF);

    // Output character directly to stdout
    putchar(c);
    fflush(stdout);  // Ensure immediate output like f_dot
}

// KEY ( -- char )  Input character from the user input device
void f_key(word_t* self) {
    (void)self;

    // Read one character from stdin
    int c = getchar();

    // Handle EOF or error conditions
    if (c == EOF) {
        c = 0;  // Push null character on EOF
    }

    // Push character value onto stack (extend to cell size)
    data_push((cell_t)(c & 0xFF));
}

// TYPE ( c-addr u -- )  Output u characters from string at c-addr
void f_type(word_t* self) {
    (void)self;

    cell_t u = data_pop();           // Character count
    forth_addr_t c_addr = (forth_addr_t)data_pop();  // String address

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

// Return stack operations - essential for colon definitions and mixed-precision arithmetic

// >R ( x -- ) ( R: -- x )  Transfer x from data stack to return stack
void f_to_r(word_t* self) {
    (void)self;

    cell_t x = data_pop();
    return_push(x);
}

// R> ( -- x ) ( R: x -- )  Transfer x from return stack to data stack
void f_r_from(word_t* self) {
    (void)self;

    cell_t x = return_pop();
    data_push(x);
}

// R@ ( -- x ) ( R: x -- x )  Copy top of return stack to data stack
void f_r_fetch(word_t* self) {
    (void)self;

    // Make sure there's something to peek at
    assert(return_depth() > 0);

    // Peek at top of return stack without removing it
    cell_t x = return_stack[return_stack_ptr - 1];
    data_push(x);
}

// M* ( n1 n2 -- d )  Multiply n1 by n2 giving signed double-cell product d
void f_m_star(word_t* self) {
    (void)self;

    cell_t n2 = data_pop();
    cell_t n1 = data_pop();

    // Use 64-bit arithmetic to handle the full range without overflow
    int64_t product = (int64_t)n1 * (int64_t)n2;

    // Push as double-cell: low cell first, then high cell
    // This follows ANS Forth convention for double-cell numbers
    data_push((cell_t)(product & 0xFFFFFFFF));        // low 32 bits
    data_push((cell_t)((product >> 32) & 0xFFFFFFFF)); // high 32 bits
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
    create_primitive_word("SWAP", f_swap);
    create_primitive_word("ROT", f_rot);
    create_primitive_word("PICK", f_pick);
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

    // Create STATE variable (0 = interpret, -1 = compile)
    state_ptr = create_variable_word("STATE", 0);

    create_primitive_word(":", f_colon);
    create_immediate_primitive_word(";", f_semicolon);
    create_primitive_word("EXIT", f_exit);

	#ifdef FORTH_DEBUG_ENABLED
    create_primitive_word("DEBUG-ON", f_debug_on);
    create_primitive_word("DEBUG-OFF", f_debug_off);
	#endif

    #ifdef FORTH_ENABLE_TESTS
    create_primitive_word("TEST", f_test);
    #endif
}

// Built-in Forth definitions (created after primitives are available)
static const char* builtin_definitions[] = {
    // Stack manipulation words
    ": DUP 0 PICK ;",
    ": OVER 1 PICK ;",
    ": 2DUP OVER OVER ;",
    ": NIP SWAP DROP ;",
    ": TUCK SWAP OVER ;",
    ": 2DROP DROP DROP ;",

    // Constants
    ": TRUE -1 ;",
    ": FALSE 0 ;",

    // Basic math
    ": NEGATE 0 SWAP - ;",
    ": 1+ 1 + ;",
    ": 1- 1 - ;",

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
            printf("ERROR: Built-in definition left system in compilation state: %s\n",
                   builtin_definitions[i]);
            *state_ptr = 0;  // Force back to interpretation
        }

        // Restore state if it was somehow changed
        if (saved_state == 0 && *state_ptr != 0) {
            *state_ptr = saved_state;
        }
    }

    debug("Built-in definitions complete");
}
