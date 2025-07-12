# PC version of KISForth
add_executable(kisforth
        src/main.c
        # src/pc_io.c will be added when we implement I/O
)

# Link with core library
target_link_libraries(kisforth PRIVATE kisforth_core)

# Platform-specific libraries for PC
if(UNIX AND NOT APPLE)
    # Linux - might need readline later
    # target_link_libraries(kisforth PRIVATE readline)
endif()

if(APPLE)
    # macOS specific settings if needed
endif()

if(WIN32)
    # Windows specific settings if needed
endif()

# Set executable properties
set_target_properties(kisforth PROPERTIES
        OUTPUT_NAME "kisforth"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
