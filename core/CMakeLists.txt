# Core KISForth library - portable Forth interpreter
add_library(kisforth_core STATIC
        src/memory.c
        src/stack.c
        src/nucleus.c
        src/dictionary.c
        src/text.c
        src/debug.c
        src/repl.c
)

# Conditionally add test system
if (ENABLE_TESTS)
    target_sources(kisforth_core PRIVATE src/test.c)
    target_compile_definitions(kisforth_core PUBLIC FORTH_ENABLE_TESTS=1)
    message(STATUS "Unit testing enabled")
endif ()

# Conditionally add tool system
if (ENABLE_TOOLS)
    target_sources(kisforth_core PRIVATE src/tools.c)
    target_compile_definitions(kisforth_core PUBLIC FORTH_ENABLE_TOOLS=1)
    message(STATUS "Programming tools word set enabled")
endif ()

# Conditionally add floating point system
if (ENABLE_FLOATING)
    target_sources(kisforth_core PRIVATE src/floating.c)
    target_compile_definitions(kisforth_core PUBLIC FORTH_ENABLE_FLOATING=1)
    message(STATUS "Floating point word set enabled")
endif ()

# Include directories
target_include_directories(kisforth_core
        PUBLIC include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include  # For generated version.h
        PRIVATE src
)

# Compiler-specific settings for core library
target_compile_options(kisforth_core PRIVATE
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wold-style-definition
)

# Platform-specific definitions
if (BUILD_FOR_PICO)
    target_compile_definitions(kisforth_core PUBLIC
            FORTH_TARGET_PICO=1
            FORTH_MEMORY_SIZE=32768  # Smaller memory for Pico
    )
else ()
    target_compile_definitions(kisforth_core PUBLIC
            FORTH_TARGET_HOST=1
    )
endif ()

# Generate version header from template
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/version.h"
)
