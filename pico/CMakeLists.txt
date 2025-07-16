# Pico executable
add_executable(kisforth_pico
        src/main.c
)

# Link against core library and Pico SDK
target_link_libraries(kisforth_pico
        kisforth_core
        pico_stdlib
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
