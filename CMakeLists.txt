set(KISFORTH_VERSION "0.0.1")

cmake_minimum_required(VERSION 3.13)

# Platform selection
option(BUILD_FOR_PICO "Build for Raspberry Pi Pico" OFF)

# Extension options
option(ENABLE_TESTS "Enable unit testing system" ON)  # Default ON for development
option(ENABLE_DEBUG "Enable debug output system" ON)  # OFF by default for production
option(ENABLE_FLOATING "Enable floating point word set" ON)
option(ENABLE_TOOLS "Enable programming tools word set" ON)  # Default ON for development

# Add after the existing platform selection options
option(BUILD_FOR_WINDOWS "Cross-compile for Windows" OFF)

if (BUILD_FOR_WINDOWS)
    # Windows cross-compilation setup
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

    # Set static linking to avoid DLL dependencies
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")

    project(kisforth VERSION ${KISFORTH_VERSION} LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    message(STATUS "Cross-compiling for Windows")
elseif (BUILD_FOR_PICO)
    # Pico-specific setup
    set(PICO_BOARD pico_w)
    include(pico/pico_sdk_import.cmake)
    project(kisforth VERSION ${KISFORTH_VERSION} LANGUAGES C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    pico_sdk_init()
    message(STATUS "Building for Raspberry Pi Pico")
else ()
    # *nix (Linux, macOS, BSD, etc.)
    project(kisforth VERSION ${KISFORTH_VERSION} LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    message(STATUS "Building for *nix development")
endif ()

# Compiler settings - use GCC for consistency
if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(WARNING "KISForth is designed for GCC. Other compilers may not work correctly.")
endif ()

# Debug/Release configuration
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# Common compiler flags (only for nix builds - Pico SDK manages its own flags)
if (NOT BUILD_FOR_PICO)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif ()

# Debug system configuration
if (ENABLE_DEBUG)
    add_compile_definitions(FORTH_DEBUG_ENABLED)
    message(STATUS "Debug output system enabled")
endif ()

# Core library (always built)
add_subdirectory(interpreter)

# Platform-specific executables
if (BUILD_FOR_PICO)
    add_subdirectory(pico)
elseif (BUILD_FOR_WINDOWS)
    add_subdirectory(windows)
else ()
    add_subdirectory(nix)
endif ()

# Status summary
message(STATUS "Build configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Target: ${BUILD_FOR_PICO}")
message(STATUS "  Extensions: Floating=${ENABLE_FLOATING}, Tools=${ENABLE_TOOLS}, Tests=${ENABLE_TESTS}, Debug=${ENABLE_DEBUG}")