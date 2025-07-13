# KISForth

A minimal but correct ANS Forth implementation in portable C.

## Philosophy

KISForth (Keep It Simple Forth) prioritizes simplicity and correctness over performance. The implementation focuses on:

- **Minimal C primitives**: Only essential operations are implemented in C, with higher-level words defined in terms of these primitives
- **Portable design**: Carefully chosen types and abstractions ensure immediate portability to 32-bit microcontrollers like the Raspberry Pi Pico
- **Standards compliance**: Implements the ANS Forth required word set plus floating-point extensions
- **Clean architecture**: Separation between core interpreter and platform-specific code

## Features

- ANS Forth required word set implementation
- Floating-point word set support
- 32-bit integer arithmetic
- Cross-platform build system with CMake
- Shared core library with platform-specific executables
- Ready for embedded deployment (tested on Raspberry Pi Pico)

## Architecture

```
kisforth/
├── core/           # Shared Forth interpreter core
├── pc/             # PC-specific application and primitives
├── pico/           # Raspberry Pi Pico application and primitives
├── CMakeLists.txt  # Root build configuration
└── README.md
```

The core library contains the platform-independent Forth interpreter, while `pc/` and `pico/` directories contain platform-specific applications and primitive implementations.

## Building

### Prerequisites

- CMake 3.10 or higher
- GCC (for consistency with Pico's arm-none-eabi-gcc)
- For Pico builds: Pico SDK

### PC Development Build (Default)

```bash
mkdir build
cd build
cmake ..
make
```

### Raspberry Pi Pico Build

```bash
mkdir build-pico
cd build-pico
cmake -DBUILD_FOR_PICO=ON ..
make
```

This generates `.uf2` files suitable for flashing to the Pico.

## Development Environment

- **OS**: Ubuntu (recommended for development)
- **Compiler**: GCC
- **Language**: C (no C++)
- **Integer size**: 32-bit
- **Address size**: 32-bit (for embedded compatibility)

## Usage

*[Usage instructions will be added as the implementation progresses]*

## Implementation Status

- [ ] Core interpreter structure
- [ ] Basic stack operations
- [ ] Dictionary management
- [ ] Word compilation and execution
- [ ] Required word set
- [ ] Floating-point word set
- [ ] PC platform support
- [ ] Pico platform support

## Contributing

This project emphasizes simplicity and correctness. When contributing:

1. Prioritize clear, readable C code over optimization
2. Maintain portability across 32-bit platforms
3. Implement higher-level functionality in Forth rather than C where possible
4. Follow ANS Forth standards

## License

This project is released into the public domain under [The Unlicense](https://unlicense.org/).

