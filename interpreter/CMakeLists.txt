# KISForth interpreter library - portable Forth interpreter
add_library(kisforth_interpreter STATIC
        src/core.c
        src/debug.c
        src/dictionary.c
        src/error.c
        src/memory.c
        src/repl.c
        src/stack.c
        src/text.c
)

# Conditionally add test system
if (ENABLE_TESTS)
    target_sources(kisforth_interpreter PRIVATE src/test.c)
    target_compile_definitions(kisforth_interpreter PUBLIC FORTH_ENABLE_TESTS=1)
    message(STATUS "Unit testing enabled")
endif ()

# Conditionally add tool system
if (ENABLE_TOOLS)
    target_sources(kisforth_interpreter PRIVATE src/tools.c)
    target_compile_definitions(kisforth_interpreter PUBLIC FORTH_ENABLE_TOOLS=1)
    message(STATUS "Programming tools word set enabled")
endif ()

# Conditionally add floating point system
if (ENABLE_FLOATING)
    target_sources(kisforth_interpreter PRIVATE src/floating.c)
    target_compile_definitions(kisforth_interpreter PUBLIC FORTH_ENABLE_FLOATING=1)
    message(STATUS "Floating point word set enabled")
endif ()

# Include directories
target_include_directories(kisforth_interpreter
        PUBLIC include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include  # For generated version.h
        PRIVATE src
)

# Compiler-specific settings for interpreter library
target_compile_options(kisforth_interpreter PRIVATE
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wold-style-definition
)

# Platform-specific definitions
if (BUILD_FOR_PICO)
    target_compile_definitions(kisforth_interpreter PUBLIC
            FORTH_TARGET_PICO=1
            FORTH_MEMORY_SIZE=32768  # Smaller memory for Pico
    )
else ()
    target_compile_definitions(kisforth_interpreter PUBLIC
            FORTH_TARGET_HOST=1
    )
endif ()

# Generate version header from template
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
)
