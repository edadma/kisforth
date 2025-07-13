#ifndef FORTH_DEBUG_H
#define FORTH_DEBUG_H

#include <stdbool.h>

// Debug module - zero overhead when disabled at compile time
// Enable with -DFORTH_DEBUG_ENABLED in CMake

#ifdef FORTH_DEBUG_ENABLED
    // When debugging is enabled, we have runtime on/off control
    extern bool debug_enabled;
void debug_raw(const char* fmt, ...);

// Macro that checks runtime flag and calls debug_raw with automatic newline
#define debug(fmt, ...) do { \
if (debug_enabled) { \
debug_raw(fmt, ##__VA_ARGS__); \
} \
} while(0)
#else
// When debugging is disabled at compile time, debug() becomes a complete no-op
#define debug(fmt, ...) ((void)0)
#endif

// Core debug interface - always available regardless of compile-time flags
void debug_init(void);
void debug_on(void);
void debug_off(void);
bool debug_is_enabled(void);

#endif // FORTH_DEBUG_H