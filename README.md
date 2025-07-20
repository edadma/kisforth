# KISForth

A minimal but correct ANS Forth implementation in portable C.

## Philosophy

KISForth (Keep It Simple Forth) prioritizes simplicity and correctness over performance. The implementation focuses on:

- **Minimal C primitives**: Only essential operations are implemented in C, with higher-level words defined in terms of
  these primitives
- **Portable design**: Carefully chosen types and abstractions ensure immediate portability to 32-bit microcontrollers
  like the Raspberry Pi Pico
- **Standards compliance**: Implements the ANS Forth required word set plus floating-point extensions
- **Clean architecture**: Separation between interpreter core and platform-specific code

## Quick Start

Pre-built executables are included in the repository root:

- **Linux**: `./kisforth-v0.0.1-linux`
- **Windows**: `kisforth-v0.0.1-windows.exe`
- **Raspberry Pi Pico**: `kisforth-v0.0.1-pico.uf2` (drag to Pico in bootloader mode)

Or build from source (see [Building](#building) section).

## Features

- **ANS Forth Core**: Complete implementation of the ANS Forth required word set
- **Floating-point support**: IEEE 754 double-precision floating-point word set
- **Programming tools**: `.S`, `WORDS`, `DUMP`, `SEE`, and other development aids
- **32-bit architecture**: Consistent 32-bit integers and addresses across platforms
- **Cross-platform build**: CMake-based build system with platform selection
- **Embedded ready**: Tested on Raspberry Pi Pico with 32KB memory footprint
- **Debug system**: Optional runtime debug output with zero overhead when disabled
- **Unit tests**: Comprehensive test suite for validation
- **Timer interrupts**: Hardware timer support on Pico with isolated execution contexts
- **WiFi support**: Network connectivity and LED control on Pico W

## Architecture

```
kisforth/
├── interpreter/           # Shared Forth interpreter core
│   ├── src/
│   │   ├── forth.c        # Core execution engine and context management
│   │   ├── core.c         # ANS Forth Core word set
│   │   ├── dictionary.c   # Dictionary management and word lookup
│   │   ├── text.c         # Text interpreter and input processing
│   │   ├── memory.c       # Virtual memory management
│   │   ├── stack.c        # Data and return stack operations
│   │   ├── floating.c     # Floating-point word set
│   │   ├── tools.c        # Programming tools word set
│   │   ├── test.c         # Unit testing framework
│   │   ├── repl.c         # Read-eval-print loop
│   │   └── ...            # Additional core modules
│   └── include/           # Public headers
├── shared/                # Shared application code
│   ├── src/startup.c      # System initialization
│   └── include/           # Shared headers
├── nix/                   # Unix/Linux/macOS platform
│   └── src/
│       ├── main.c         # Platform entry point
│       └── key_input.c    # Platform-specific input handling
├── windows/               # Windows platform
│   └── src/
│       ├── main.c         # Platform entry point
│       └── key_input.c    # Platform-specific input handling
├── pico/                  # Raspberry Pi Pico platform
│   ├── src/
│   │   ├── main.c         # Platform entry point
│   │   ├── systick.c      # Timer interrupt system
│   │   ├── wifi_support.c # WiFi connectivity
│   │   ├── wifi_words.c   # WiFi-related Forth words
│   │   └── stdlib_words.c # GPIO and timing words
│   └── include/           # Platform headers
├── CMakeLists.txt         # Root build configuration
├── kisforth-v0.0.1-linux          # Pre-built Linux executable
├── kisforth-v0.0.1-windows.exe    # Pre-built Windows executable
├── kisforth-v0.0.1-pico.uf2       # Pre-built Pico firmware
└── README.md
```

## Implementation Status

### Core Word Set (✅ Complete)

- **Arithmetic**: `+`, `-`, `*`, `/`, `MOD`, `ABS`, `NEGATE`, `M*`, `SM/REM`, `FM/MOD`
- **Stack manipulation**: `DROP`, `DUP`, `SWAP`, `ROT`, `OVER`, `PICK`, `ROLL`
- **Memory access**: `@`, `!`, `C@`, `C!`, `+!`, `2@`, `2!`
- **Comparison**: `=`, `<`, `>`, `0=`, `0<`, `U<`
- **Logic**: `AND`, `OR`, `XOR`, `INVERT`
- **Memory allocation**: `HERE`, `ALLOT`, `,`, `C,`, `ALIGN`, `ALIGNED`
- **I/O**: `EMIT`, `KEY`, `TYPE`, `.`, `CR`, `SPACE`, `SPACES`
- **Return stack**: `>R`, `R>`, `R@`
- **Control flow**: `IF`/`THEN`/`ELSE`, `DO`/`LOOP`, `BEGIN`/`WHILE`/`REPEAT`
- **Compilation**: `:`, `;`, `IMMEDIATE`, `[`, `]`, `LITERAL`
- **Dictionary**: `'`, `EXECUTE`, `FIND`, `WORD`, `CREATE`, `DOES>`
- **Variables**: `VARIABLE`, `CONSTANT`, `STATE`, `BASE`
- **Strings**: `S"`, `."`, `COUNT`, `EVALUATE`
- **Input system**: `SOURCE`, `>IN`, `ACCEPT`, `QUIT`

### Optional Word Sets (✅ Complete)

- **Floating-point**: `F+`, `F-`, `F*`, `F/`, `F.`, `FDROP`, `FDUP`, `FLIT`
- **Programming tools**: `.S`, `WORDS`, `DUMP`, `?`, `SEE` (stub), `UNUSED`
- **System**: `BYE`, `ABORT`, `ABORT"`

### Platform-Specific Extensions

#### Raspberry Pi Pico (✅ Complete)

- **GPIO control**: `GPIO-INIT`, `GPIO-OUT`, `GPIO-IN`, `GPIO-PUT`, `GPIO-GET`
- **Timing**: `MS`, `US`, `TICKS`
- **Timer interrupts**: `SYSTICK-START`, `SYSTICK-STOP`
- **WiFi support**: `WIFI-INIT`, `WIFI-CONNECT`, `WIFI-STATUS`, `WIFI-CONNECTED?`, `WIFI-DISCONNECT`
- **LED control**: `LED-ON`, `LED-OFF`, `LED-TOGGLE`
- **Multi-context**: Isolated execution contexts for main program and timer interrupts

### Development Features (✅ Complete)

- **Unit testing**: Comprehensive test framework with `TEST` command
- **Debug system**: Runtime debug output (`DEBUG-ON`/`DEBUG-OFF`)
- **Memory inspection**: `DUMP`, `WORDS`, `.S` for examining system state
- **Cross-compilation**: Support for Linux, Windows, and Pico targets

## Building

### Prerequisites

- CMake 3.13 or higher
- GCC (recommended for consistency with Pico's arm-none-eabi-gcc)
- For Pico builds: [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- For Windows cross-compilation: MinGW-w64 (`apt install gcc-mingw-w64`)

### Quick Build (Linux/macOS)

```bash
mkdir build && cd build
cmake ..
make
```

### Platform-Specific Builds

#### Raspberry Pi Pico

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
mkdir build-pico && cd build-pico
cmake -DBUILD_FOR_PICO=ON ..
make
```

This generates a `.uf2` file that can be dragged onto the Pico when in bootloader mode.

#### Windows Cross-Compilation (from Linux)

```bash
mkdir build-windows && cd build-windows
cmake -DBUILD_FOR_WINDOWS=ON ..
make
```

### Build Options

- `BUILD_FOR_PICO=ON` - Build for Raspberry Pi Pico
- `BUILD_FOR_WINDOWS=ON` - Cross-compile for Windows (requires MinGW-w64)
- `ENABLE_DEBUG=ON` - Enable debug output (default: ON)
- `ENABLE_TESTS=ON` - Enable unit tests (default: ON)
- `ENABLE_FLOATING=ON` - Enable floating-point word set (default: ON)
- `ENABLE_TOOLS=ON` - Enable programming tools (default: ON)
- `COPY_EXECUTABLES_TO_ROOT=ON` - Copy built executables to repository root (default: ON)

### Debug Build

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_DEBUG=ON ..
make
```

## Usage

### Interactive Mode

```bash
./kisforth
```

```forth
KISForth v0.0.1 - Nix Development Version
Memory: 65536 bytes allocated
Extensions: [FLOAT] [TOOLS] [TESTS] [DEBUG]

Type 'WORDS' to see available commands
Type 'TEST' to run unit tests
Type 'BYE' to exit

Ready.
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

Runs the built-in unit test suite, validating core functionality.

### Floating-Point Support

```forth
ok> 3.14159 2.71828 F+
ok> F.
5.85987 ok>

ok> 1.0 2.0 F/ F.
0.5 ok>

ok> : CIRCLE-AREA ( radius -- area ) FDUP F* 3.14159 F* ;
ok> 5.0 CIRCLE-AREA F.
78.5398 ok>
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

ok> UNUSED .
49152 ok>
```

### Pico Platform Features

#### GPIO Control

```forth
\ Initialize and control onboard LED (GPIO 25)
25 GPIO-INIT 25 GPIO-OUT    \ Initialize as output
25 1 GPIO-PUT               \ Turn LED on
25 0 GPIO-PUT               \ Turn LED off

\ Read external input
2 GPIO-INIT 2 GPIO-IN       \ Initialize GPIO 2 as input
2 GPIO-GET .                \ Read current state
```

#### Timing and Sleep

```forth
1000 MS                     \ Sleep 1 second
500 US                      \ Sleep 500 microseconds
TICKS .                     \ Show milliseconds since boot
```

#### Timer Interrupt System

The Pico platform includes sophisticated timer interrupt support that allows Forth words to execute at regular intervals
without interfering with main program execution:

```forth
\ Simple heartbeat every 1000ms
: HEARTBEAT ." tick " TICKS . CR ;
1000 ' HEARTBEAT SYSTICK-START

\ Blink onboard LED every 500ms
25 GPIO-INIT 25 GPIO-OUT
VARIABLE LED-STATE
: BLINK 25 LED-STATE @ 0= DUP LED-STATE ! GPIO-PUT ;
500 ' BLINK SYSTICK-START

\ Stop timer
SYSTICK-STOP
```

The timer system demonstrates true multi-tasking capability - you can type and execute Forth commands interactively
while timer interrupts continue to fire in the background.

#### WiFi Support (Pico W)

```forth
\ Initialize WiFi
WIFI-INIT .                 \ Returns true if successful

\ Connect to network (example with string literals)
S" MyNetwork" S" MyPassword" WIFI-CONNECT .

\ Check connection status
WIFI-CONNECTED? .           \ Returns true if connected
WIFI-STATUS                 \ Print detailed status

\ Control onboard LED (requires WiFi init)
LED-ON                      \ Turn on onboard LED
LED-OFF                     \ Turn off onboard LED
LED-TOGGLE                  \ Toggle LED state
```

## Technical Details

### Memory Architecture

- **Virtual memory**: All Forth addresses are 32-bit offsets into a unified memory space
- **Portability**: Same address space on both 32-bit and 64-bit host systems
- **Memory sizes**: 64KB on development platforms, 32KB on Pico
- **Cell size**: 32-bit signed integers throughout

### Context System

KISForth uses a context-based execution model that enables advanced features like timer interrupts:

- **Main context**: Interactive REPL and user programs
- **Interrupt contexts**: Isolated execution for timer callbacks
- **Separate stacks**: Each context has its own data, return, and float stacks
- **Memory isolation**: Contexts share dictionary but have separate transient areas

### Build System Features

- **Platform detection**: Automatic selection between nix/Windows/Pico targets
- **Optional features**: Compile-time selection of word sets and debug features
- **Cross-compilation**: Full Windows cross-compilation support from Linux
- **Executable generation**: Automatic copying of built executables to repository root
- **Version management**: Unified version numbering across all platforms

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

## Version History

### v0.0.1 (Current)

- Complete ANS Forth core word set implementation
- Floating-point word set with IEEE 754 double precision
- Programming tools word set
- Multi-platform build system (Linux, Windows, Pico)
- Timer interrupt system for Pico
- WiFi support for Pico W
- Comprehensive unit test framework
- Debug system with runtime control
- Pre-built executables for all platforms

## License

This project is released into the public domain under [The Unlicense](https://unlicense.org/).

Use it freely for any purpose, commercial or non-commercial, with no attribution required.

## Acknowledgments

Based on the ANS Forth Standard (ANSI X3.215-1994) and inspired by the elegance of Chuck Moore's original Forth
implementations.