# KISForth

A minimal but correct ANS Forth implementation in portable C.

## Philosophy

KISForth (Keep It Simple Forth) prioritizes simplicity and correctness over performance. The implementation focuses on:

- **Minimal C primitives**: Only essential operations are implemented in C, with higher-level words defined in terms of
  these primitives
- **Portable design**: Carefully chosen types and abstractions ensure immediate portability to 32-bit microcontrollers
  like the Raspberry Pi Pico
- **Standards compliance**: Implements the ANS Forth required word set plus floating-point extensions
- **Clean architecture**: Separation between interpreter and platform-specific code

## Features

- **ANS Forth Core**: Implements most of the ANS Forth required word set
- **Floating-point support**: Complete floating-point word set with double-precision arithmetic
- **Programming tools**: `.S`, `WORDS`, `DUMP`, and other development aids
- **32-bit architecture**: Consistent 32-bit integers and addresses across platforms
- **Cross-platform build**: CMake-based build system with platform selection
- **Embedded ready**: Tested on Raspberry Pi Pico with 32KB memory footprint
- **Debug system**: Optional runtime debug output with zero overhead when disabled
- **Unit tests**: Comprehensive test suite for validation

## Architecture

```
kisforth/
├── interpreter/           # Shared Forth interpreter
│   ├── src/
│   │   ├── core.c         # ANS Forth Core word set
│   │   ├── dictionary.c   # Dictionary management
│   │   ├── text.c         # Text interpreter
│   │   ├── memory.c       # Memory management
│   │   ├── stack.c        # Stack operations
│   │   ├── floating.c     # Floating-point word set
│   │   ├── tools.c        # Programming tools word set
│   │   └── test.c         # Unit testing framework
│   └── include/           # Public headers
├── nix/                   # Nix development platform
│   └── src/main.c         # Nix application entry point
├── pico/                  # Raspberry Pi Pico platform
│   └── src/main.c         # Pico application entry point
├── CMakeLists.txt         # Root build configuration
└── README.md
```

## Current Implementation Status

### Core Word Set

- ✅ **Arithmetic**: `+`, `-`, `*`, `/`, `MOD`, `ABS`, `NEGATE`, `M*`, `SM/REM`, `FM/MOD`
- ✅ **Stack manipulation**: `DROP`, `DUP`, `SWAP`, `ROT`, `OVER`, `PICK`, `ROLL`
- ✅ **Memory access**: `@`, `!`, `C@`, `C!`, `+!`, `2@`, `2!`
- ✅ **Comparison**: `=`, `<`, `>`, `0=`, `0<`, `U<`
- ✅ **Logic**: `AND`, `OR`, `XOR`, `INVERT`
- ✅ **Memory allocation**: `HERE`, `ALLOT`, `,`, `C,`, `ALIGN`, `ALIGNED`
- ✅ **I/O**: `EMIT`, `KEY`, `TYPE`, `.`, `CR`, `SPACE`, `SPACES`
- ✅ **Return stack**: `>R`, `R>`, `R@`
- ✅ **Control flow**: `IF`/`THEN`/`ELSE`, `DO`/`LOOP`, `BEGIN`/`WHILE`/`REPEAT`
- ✅ **Compilation**: `:`, `;`, `IMMEDIATE`, `[`, `]`, `LITERAL`
- ✅ **Dictionary**: `'`, `EXECUTE`, `FIND`, `WORD`, `CREATE`, `DOES>`
- ✅ **Variables**: `VARIABLE`, `CONSTANT`, `STATE`, `BASE`
- ✅ **Strings**: `S"`, `."`, `COUNT`, `EVALUATE`
- ✅ **Input system**: `SOURCE`, `>IN`, `ACCEPT`, `QUIT`

### Optional Word Sets

- ✅ **Floating-point**: `F+`, `F-`, `F*`, `F/`, `F.`, `FDROP`, `FDUP`, `FLIT`
- ✅ **Programming tools**: `.S`, `WORDS`, `DUMP`, `?`, `SEE` (stub)
- ✅ **System**: `BYE`, `ABORT`, `ABORT"`

### Platform Support

- ✅ **Nix development**: Linux, macOS, Windows (MinGW cross-compilation)
- ✅ **Raspberry Pi Pico**: Full support with USB serial I/O
- ✅ **Memory management**: 64KB virtual memory (host), 32KB (Pico)

## Building

### Prerequisites

- CMake 3.13 or higher
- GCC (recommended for consistency with Pico's arm-none-eabi-gcc)
- For Pico builds: [Pico SDK](https://github.com/raspberrypi/pico-sdk)

### Nix Development Build (Default)

```bash
mkdir build
cd build
cmake ..
make
```

### Raspberry Pi Pico Build

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir build-pico
cd build-pico
cmake -DBUILD_FOR_PICO=ON ..
make
```

This generates a `.uf2` file that can be dragged onto the Pico when in bootloader mode.

### Debug Build

```bash
mkdir build-debug
cd build-debug
cmake -DENABLE_DEBUG=ON ..
make
```

### Build Options

- `BUILD_FOR_PICO=ON` - Build for Raspberry Pi Pico
- `BUILD_FOR_WINDOWS=ON` - Cross-compile for Windows
- `ENABLE_DEBUG=ON` - Enable debug output (default: ON)
- `ENABLE_TESTS=ON` - Enable unit tests (default: ON)
- `ENABLE_FLOATING=ON` - Enable floating-point word set (default: ON)
- `ENABLE_TOOLS=ON` - Enable programming tools (default: ON)

## Usage

### Interactive Mode

```bash
./kisforth
```

```forth
KISForth v0.0.1 - Nix Development Version
Memory size: 65536 bytes
ok> 42 43 +
ok> .
85 ok> 

ok> : SQUARE DUP * ;
ok> 7 SQUARE .
49 ok>

ok> WORDS
Dictionary words:
SQUARE       ;            :            EXIT         
IMMEDIATE    STATE        BASE         PAD          
CREATE       VARIABLE     0BRANCH      BRANCH       
[']          +            -            *            
/            DROP         SOURCE       >IN          
... (many more words)
```

### Test Mode

```bash
./kisforth test
```

Runs the built-in unit test suite.

### Floating-Point Support

```forth
ok> 3.14159 2.71828 F+
ok> F.
5.85987 ok>

ok> 1.0 2.0 F/ F.
0.5 ok>
```

### Programming Tools

```forth
ok> 1 2 3 .S
<3> 1 2 3 
ok> 

ok> HERE 16 DUMP
DUMP 0000A120 (16 bytes):
0000A120: 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 |................|
ok>
```

## Development Environment

- **OS**: Ubuntu/Linux recommended for development
- **Compiler**: GCC (for consistency with embedded toolchain)
- **Language**: C11 (no C++)
- **Integer size**: 32-bit cells
- **Address size**: 32-bit (for embedded compatibility)
- **Memory model**: Virtual memory with unified address space

## Design Principles

1. **Simplicity over performance**: Code should be readable and maintainable
2. **Minimal C primitives**: Implement only what cannot be expressed in Forth
3. **Standards compliance**: Follow ANS Forth specifications precisely
4. **Platform independence**: Use careful abstractions for portability
5. **Zero-overhead features**: Optional features have no cost when disabled

## Contributing

This project emphasizes clarity and correctness. When contributing:

1. **Follow the design philosophy**: Prefer simple, clear code over optimizations
2. **Maintain portability**: Test on both host and Pico platforms
3. **Implement in Forth when possible**: Add new words as colon definitions rather than C primitives
4. **Follow ANS Forth**: Consult the standard for correct behavior
5. **Add tests**: Include unit tests for new functionality
6. **Document decisions**: Use clear comments explaining design choices

### Code Style

- C11 standard with GCC extensions
- 4-space indentation
- Clear variable names (avoid abbreviations)
- Comprehensive error checking
- Debug output for tracing execution

## License

This project is released into the public domain under [The Unlicense](https://unlicense.org/).

Use it freely for any purpose, commercial or non-commercial, with no attribution required.

## Acknowledgments

Based on the ANS Forth Standard (ANSI X3.215-1994) and inspired by the elegance of Chuck Moore's original Forth
implementations.