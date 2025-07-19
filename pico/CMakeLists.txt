# Include shared application code
include(../shared/CMakeLists.txt)

# Pico executable
add_executable(kisforth_pico
        src/main.c
        ${KISFORTH_SHARED_SOURCES}
)

# Add shared includes (add this block)
target_include_directories(kisforth_pico PRIVATE
        ${KISFORTH_SHARED_INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link against interpreter library and Pico SDK
target_link_libraries(kisforth_pico
        kisforth_interpreter
        pico_stdlib
        pico_cyw43_arch_none
        pico_lwip_nosys
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
        src/wifi_support.c
)
