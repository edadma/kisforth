# Include shared application code
include(../shared/CMakeLists.txt)

# Windows version of KISForth
add_executable(kisforth
        src/main.c
        ${KISFORTH_SHARED_SOURCES}
)

# Add shared includes
target_include_directories(kisforth PRIVATE
        ${KISFORTH_SHARED_INCLUDES}
)

# Link with interpreter library
target_link_libraries(kisforth PRIVATE kisforth_interpreter)

# Windows-specific settings
set_target_properties(kisforth PROPERTIES
        OUTPUT_NAME "kisforth"
        SUFFIX ".exe"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_sources(kisforth PRIVATE
        src/key_input.c
)