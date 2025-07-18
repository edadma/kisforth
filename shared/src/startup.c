#include "startup.h"
#include "stack.h"
#include "debug.h"
#include "floating.h"
#include "memory.h"
#include "dictionary.h"
#include "version.h"

void forth_system_init(void) {
    // Initialize core systems in dependency order
    stack_init();

#ifdef FORTH_DEBUG_ENABLED
    debug_init();
#endif

#ifdef FORTH_ENABLE_FLOATING
    float_stack_init();
#endif

    input_system_init();
    dictionary_init();
}

void print_startup_banner(const char* platform_name) {
    printf("KISForth v%s - %s Version\n", KISFORTH_VERSION_STRING, platform_name);
    printf("Memory: %d bytes allocated\n", FORTH_MEMORY_SIZE);
    print_extensions_list();

#ifdef FORTH_TARGET_PICO
    printf("Connect via USB serial\n");
#endif

    printf("\nType 'WORDS' to see available commands\n");

#ifdef FORTH_ENABLE_TESTS
    printf("Type 'TEST' to run unit tests\n");
#endif

#ifdef FORTH_TARGET_HOST
    printf("Type 'BYE' to exit\n");
#endif

    printf("\nReady.\n");
}

void print_extensions_list(void) {
    printf("Extensions:");

#ifdef FORTH_ENABLE_FLOATING
    printf(" [FLOAT]");
#endif

#ifdef FORTH_ENABLE_TOOLS
    printf(" [TOOLS]");
#endif

#ifdef FORTH_ENABLE_TESTS
    printf(" [TESTS]");
#endif

#ifdef FORTH_DEBUG_ENABLED
    printf(" [DEBUG]");
#endif

    printf("\n");
}