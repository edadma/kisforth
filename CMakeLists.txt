cmake_minimum_required(VERSION 3.16)
project(kisforth VERSION 0.0.1 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Platform selection
option(BUILD_FOR_PICO "Build for Raspberry Pi Pico" OFF)

# Extension options
option(ENABLE_FLOATING "Enable floating point word set" OFF)
option(ENABLE_DOUBLE "Enable double number extension" OFF)
option(ENABLE_TESTS "Enable unit testing system" ON)  # Default ON for development
option(ENABLE_DEBUG "Enable debug output system" OFF)  # OFF by default for production

# Compiler settings - use GCC for consistency
if(NOT CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message(WARNING "KISForth is designed for GCC. Other compilers may not work correctly.")
endif()

# Debug/Release configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Common compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# Debug system configuration
if(ENABLE_DEBUG)
    add_compile_definitions(FORTH_DEBUG_ENABLED)
    message(STATUS "Debug output system enabled")
endif()

# Platform-specific configuration
if(BUILD_FOR_PICO)
    # Pico SDK setup
    if(NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH must be set for Pico builds")
    endif()

    include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
    pico_sdk_init()

    message(STATUS "Building for Raspberry Pi Pico")
else()
    message(STATUS "Building for PC development")
endif()

# Core library (always built)
add_subdirectory(core)

# Extensions (optional)
if(ENABLE_FLOATING)
    add_subdirectory(extensions/floating)
    message(STATUS "Floating point extension enabled")
endif()

if(ENABLE_DOUBLE)
    add_subdirectory(extensions/double)
    message(STATUS "Double number extension enabled")
endif()

# Platform-specific executables
if(BUILD_FOR_PICO)
    add_subdirectory(pico)
else()
    add_subdirectory(pc)
endif()

# Status summary
message(STATUS "Build configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Target: ${BUILD_FOR_PICO}")
message(STATUS "  Extensions: Floating=${ENABLE_FLOATING}, Double=${ENABLE_DOUBLE}, Tests=${ENABLE_TESTS}, Debug=${ENABLE_DEBUG}")