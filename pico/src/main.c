#include <stdio.h>
#include "pico/stdlib.h"
#include "forth.h"
#include "version.h"

int main() {
    // Initialize stdio over USB CDC
    stdio_init_all();

    // Small delay to ensure USB connection is stable
    sleep_ms(2000);

    printf("KISForth v%s - Pico Version\n", KISFORTH_VERSION_STRING);
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);

    // Initialize the Forth system
    stack_init();
    dictionary_init();
    create_all_primitives();

    // Set up Pico I/O interface (we'll create this)
    set_io_interface(get_pico_io());

    printf("Forth system initialized.\n");
    printf("Connect via USB serial (minicom, screen, etc.)\n");

    // Start interactive REPL
    forth_repl();

    return 0;
}