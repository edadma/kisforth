# KISForth Design Document

## Project Goals
- Minimal but correct ANS Forth implementation in portable C
- Focus on simplicity of code over performance
- Only implement required word set (not extensions)
- Portable between PC development and Raspberry Pi Pico deployment

## Core Data Types
```c
typedef int32_t cell_t;        // 32-bit cell
typedef uint32_t ucell_t;      // Unsigned cell
typedef uint8_t byte_t;        // Byte for C@ C! operations
typedef uint32_t forth_addr_t; // Forth address (always 32-bit)
```

## Memory Model
- **Virtual Memory**: Byte array `forth_memory[64KB]` for all Forth data
- **Addressing**: 32-bit byte offsets into virtual memory (portable across platforms)
- **Conversion**: Simple functions convert between Forth addresses and C pointers
- **Alignment**: Manual alignment for C structures when needed

## Dictionary Structure
```c
typedef struct word {
    struct word* link;          // Link to previous word (C pointer)
    char name[32];              // Word name (31 chars max per standard)
    uint8_t flags;              // Immediate flag, etc.
    void (*cfunc)(void);        // C function for ALL word types
    // Parameter field follows immediately after
} word_t;
```

## Word Execution Model
```c
typedef void (*cfunc_t)(word_t* self);
```
- **Uniform Execution**: All words execute via `word->cfunc(word)` - no special cases
- **Self-Reference**: `cfunc` receives pointer to its own word for parameter field access
- **Primitives**: `cfunc` points directly to implementation (e.g., `forth_plus`)
- **Colon Definitions**: `cfunc` points to generic `execute_colon(word_t* self)` 
- **Variables/Constants**: `cfunc` points to appropriate generic handler
- **Debugging**: Easy execution tracing since cfunc knows which word is executing

## Compilation Model
- **Token-Based**: Colon definitions compiled as arrays of 32-bit tokens
- **Tokens Are Addresses**: Word tokens are Forth addresses (offsets into memory)
- **Literals**: Built-in `LIT` word with `f_lit` cfunc reads next token as literal
- **Control Structures**: Implemented as immediate words using branching primitives
- **Example**: `: DOUBLE 2 * ;` → `[LIT_ADDR, 2, MULT_ADDR, EXIT_ADDR]`

## Project Structure
```
kisforth/
├── core/                    # Portable Forth interpreter (static library)
│   ├── include/forth.h      # Public API
│   ├── src/
│   │   ├── forth.c          # Main interpreter, initialization
│   │   ├── stack.c          # Data/return stack operations
│   │   ├── memory.c         # Virtual memory, @, !, ALLOT
│   │   ├── dictionary.c     # Word creation, lookup, FIND
│   │   ├── nucleus.c        # Arithmetic, logic primitives
│   │   ├── text.c           # Text interpreter, number conversion
│   │   ├── compiler.c       # Colon definitions, control structures
│   │   └── io_interface.c   # Platform-agnostic I/O abstraction
│   └── CMakeLists.txt
├── extensions/              # Optional word set extensions
│   ├── floating/            # Floating point extension
│   │   ├── include/floating.h
│   │   ├── src/floating.c   # F+, F*, FSIN, etc.
│   │   └── CMakeLists.txt
│   └── double/              # Double number extension (future)
│       └── ...
├── pc/                      # PC version
│   ├── src/
│   │   ├── main.c           # PC main, command line
│   │   └── pc_io.c          # readline integration
│   └── CMakeLists.txt
└── pico/                    # Pico version
    ├── src/
    │   ├── main.c           # Pico main, USB setup
    │   ├── pico_io.c        # Simple character I/O
    │   └── pico_primitives.c # GPIO, WiFi primitives
    └── CMakeLists.txt
```

## Build System
- **CMake-based** with explicit platform selection
- **PC Development**: `cmake ..` (default)
- **Pico Deployment**: `cmake -DBUILD_FOR_PICO=ON ..`
- **Optional Extensions**: `cmake -DENABLE_FLOATING=ON -DENABLE_DOUBLE=ON ..`
- **Compiler**: GCC (for consistency with Pico's arm-none-eabi-gcc)
- **Standard**: C11

## Optional Extensions
- **Modular Design**: Extensions are separate libraries that register words
- **Floating Point**: IEEE 754 float support (F+, F*, FSIN, F., etc.)
- **Registration API**: Extensions call `forth_register_primitive()` during init
- **Example**: Floating point adds separate float stack and math operations
- **Platform Support**: Extensions can be enabled/disabled per platform

## I/O Architecture
- **Abstract Interface**: Core uses function pointers for all I/O
- **Platform Implementation**: Each platform provides concrete implementations
- **PC Features**: GNU readline with history and editing
- **Pico Features**: Simple character-based input with echo

## Error Handling
- **setjmp/longjmp**: Clean abort back to REPL loop
- **Standard Compliant**: Implements ABORT and ABORT" behavior
- **Recovery Actions**: Clear stacks, reset state, continue REPL

## Memory Layout
```c
#define FORTH_MEMORY_SIZE (64 * 1024)  // 64KB virtual memory
#define DATA_STACK_SIZE 64             // Per standard minimum
#define RETURN_STACK_SIZE 48           // Per standard minimum
#define INPUT_BUFFER_SIZE 256          // Text input buffer
```

## Bootstrap Strategy
- **Hard-coded Primitives**: All required words initialized in C
- **Dictionary Creation**: Built programmatically at startup
- **Standard Compliance**: All required words present in FORTH vocabulary

## Implementation Priorities
1. Core data structures and memory management
2. Basic stack operations and arithmetic primitives
3. Dictionary creation and word lookup
4. Simple text interpreter (numbers and word execution)
5. Colon definition compilation and execution
6. Branching primitives (BRANCH, ?BRANCH, >MARK, >RESOLVE, <MARK, <RESOLVE)
7. Control structure words as immediate definitions (IF/THEN, DO/LOOP, etc.)
7. I/O primitives and REPL
8. Platform-specific features 
