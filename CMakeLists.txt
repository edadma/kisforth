cmake_minimum_required(VERSION 3.13)

# Platform selection
option(BUILD_FOR_PICO "Build for Raspberry Pi Pico" OFF)

# Extension options
option(ENABLE_TESTS "Enable unit testing system" ON)  # Default ON for development
option(ENABLE_DEBUG "Enable debug output system" OFF)  # OFF by default for production

option(ENABLE_FLOATING "Enable floating point word set" OFF)
option(ENABLE_TOOLS "Enable programming tools word set" ON)  # Default ON for development

if (BUILD_FOR_PICO)
    # Pico-specific setup
    set(PICO_BOARD pico)  # Change to pico_w if using Pico W
    include(pico/pico_sdk_import.cmake)
    project(kisforth VERSION 0.0.1 LANGUAGES C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    pico_sdk_init()
    message(STATUS "Building for Raspberry Pi Pico")
else ()
    # PC build setup
    project(kisforth VERSION 0.0.1 LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    message(STATUS "Building for PC development")
endif ()

# Compiler settings - use GCC for consistency
if (NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(WARNING "KISForth is designed for GCC. Other compilers may not work correctly.")
endif ()

# Debug/Release configuration
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

# Common compiler flags (only for PC builds - Pico SDK manages its own flags)
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
add_subdirectory(core)

# Extensions (optional)
if (ENABLE_FLOATING)
    add_subdirectory(extensions/floating)
    message(STATUS "Floating point extension enabled")
endif ()

# Platform-specific executables
if (BUILD_FOR_PICO)
    add_subdirectory(pico)
else ()
    add_subdirectory(pc)
endif ()

# Status summary
message(STATUS "Build configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Target: ${BUILD_FOR_PICO}")
message(STATUS "  Extensions: Floating=${ENABLE_FLOATING}, Tools=${ENABLE_TOOLS}, Tests=${ENABLE_TESTS}, Debug=${ENABLE_DEBUG}")