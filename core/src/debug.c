#include "debug.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef FORTH_DEBUG_ENABLED

// Runtime debug state - only exists when debugging is enabled at compile time
bool debug_enabled = false;

// Internal function that does the actual printf work with automatic newline
void debug_raw(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // Print the formatted message
    vprintf(fmt, args);

    // Always add a newline
    printf("\n");

    // Ensure output appears immediately (important for debugging)
    fflush(stdout);

    va_end(args);
}

#endif // FORTH_DEBUG_ENABLED

// Core debug interface functions - always available
// This ensures Forth words DEBUG-ON/DEBUG-OFF can always be linked

void debug_init(void) {
#ifdef FORTH_DEBUG_ENABLED
    debug_enabled = false;  // Start with debugging off
#endif
    // When debugging disabled at compile time, this is a no-op
}

void debug_on(void) {
#ifdef FORTH_DEBUG_ENABLED
    debug_enabled = true;
    printf("Debug output enabled\n");
    fflush(stdout);
#endif
    // When debugging disabled at compile time, this is a no-op
}

void debug_off(void) {
#ifdef FORTH_DEBUG_ENABLED
    debug_enabled = false;
    printf("Debug output disabled\n");
    fflush(stdout);
#endif
    // When debugging disabled at compile time, this is a no-op
}

bool debug_is_enabled(void) {
#ifdef FORTH_DEBUG_ENABLED
    return debug_enabled;
#else
    return false;  // Always disabled when not compiled in
#endif
}