#include <stdio.h>
#include "pico/stdlib.h"
#include "forth.h"
#include "memory.h"
#include "stack.h"
#include "dictionary.h"
#include "repl.h"
#include "version.h"

int main() {
    // Initialize stdio over USB CDC
    stdio_init_all();

    // Wait for USB CDC connection
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    // Give a tiny bit more time for terminal to be ready
    sleep_ms(500);

    printf("\n");
    printf("KISForth v%s - Pico Version\n", KISFORTH_VERSION_STRING);
    printf("Memory size: %d bytes\n", FORTH_MEMORY_SIZE);

    // Initialize the Forth system
    stack_init();
    input_system_init();    // Initialize input buffers in Forth memory
    dictionary_init();

    printf("Forth system initialized.\n");
    printf("Connect via USB serial (minicom, screen, etc.)\n");

    // Start interactive REPL
    repl();

    return 0;
}