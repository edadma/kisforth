# Include shared application code
include(../shared/CMakeLists.txt)

# Pico executable
add_executable(kisforth_pico
        src/main.c
        ${KISFORTH_SHARED_SOURCES}
)

# Add shared includes (add this block)
target_include_directories(kisforth_pico PRIVATE
        PUBLIC include
        ${KISFORTH_SHARED_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        PRIVATE src
)

# Link against interpreter library and Pico SDK
target_link_libraries(kisforth_pico
        kisforth_interpreter
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background #pico_cyw43_arch_none
        pico_lwip_http #pico_lwip_nosys
)

# Enable USB stdio, disable UART stdio
pico_enable_stdio_usb(kisforth_pico 1)
pico_enable_stdio_uart(kisforth_pico 0)

# Create UF2 file for easy flashing
pico_add_extra_outputs(kisforth_pico)

# Enable memory usage reporting
set_target_properties(kisforth_pico PROPERTIES
        LINK_FLAGS "-Wl,--print-memory-usage"
)

target_sources(kisforth_pico PRIVATE
        src/key_input.c
        src/stdlib_words.c
        src/systick.c
        src/wifi_support.c
        src/wifi_words.c
)

# For Pico - use a slight delay to ensure .uf2 is generated first
if (COPY_EXECUTABLES_TO_ROOT AND BUILD_FOR_PICO)
    add_custom_command(TARGET kisforth_pico POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/kisforth_pico.uf2
            ${CMAKE_SOURCE_DIR}/kisforth-v${KISFORTH_VERSION}-pico.uf2
            COMMENT "Copying UF2 file to repository root"
    )
endif ()
