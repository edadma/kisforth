# Include shared application code
include(../shared/CMakeLists.txt)

# Host version of KISForth
add_executable(kisforth
        src/main.c
        ${KISFORTH_SHARED_SOURCES}  # Add this line
)

# Add shared includes (add this block)
target_include_directories(kisforth PRIVATE
        ${KISFORTH_SHARED_INCLUDES}
)

# Link with interpreter library (this should already be there)
target_link_libraries(kisforth PRIVATE kisforth_interpreter)

# Link with interpreter library
target_link_libraries(kisforth PRIVATE kisforth_interpreter)

# Platform-specific libraries for host
if (BUILD_FOR_WINDOWS)
    # Windows-specific settings
    set_target_properties(kisforth PROPERTIES
            OUTPUT_NAME "kisforth"
            SUFFIX ".exe"
    )
elseif (UNIX AND NOT APPLE)    # Linux - might need readline later
    # target_link_libraries(kisforth PRIVATE readline)
endif ()

if (APPLE)
    # macOS specific settings if needed
endif ()

if (WIN32)
    # Windows specific settings if needed
endif ()

# Set executable properties
set_target_properties(kisforth PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
