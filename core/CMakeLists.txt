# Core KISForth library - portable Forth interpreter
add_library(kisforth_core STATIC
        src/memory.c
        src/stack.c
        src/nucleus.c
        src/dictionary.c
        src/text.c
        # Additional sources will be added as we implement them:
        # src/forth.c          # Main interpreter, initialization
        # src/compiler.c       # Colon definitions, control structures
        # src/io_interface.c   # Platform-agnostic I/O abstraction
)

# Include directories
target_include_directories(kisforth_core
        PUBLIC include
        PRIVATE src
)

# Compiler-specific settings for core library
target_compile_options(kisforth_core PRIVATE
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wold-style-definition
)

# Platform-specific definitions
if(BUILD_FOR_PICO)
    target_compile_definitions(kisforth_core PUBLIC
            FORTH_TARGET_PICO=1
            FORTH_MEMORY_SIZE=32768  # Smaller memory for Pico
    )
else()
    target_compile_definitions(kisforth_core PUBLIC
            FORTH_TARGET_PC=1
    )
endif()

# Extension support
if(ENABLE_FLOATING)
    target_compile_definitions(kisforth_core PUBLIC FORTH_ENABLE_FLOATING=1)
endif()

if(ENABLE_DOUBLE)
    target_compile_definitions(kisforth_core PUBLIC FORTH_ENABLE_DOUBLE=1)
endif()