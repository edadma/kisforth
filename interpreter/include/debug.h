#ifndef FORTH_DEBUG_H
#define FORTH_DEBUG_H

#include <stdbool.h>

#include "types.h"

// Debug module - zero overhead when disabled at compile time
// Enable with -DFORTH_DEBUG_ENABLED in CMake

#ifdef FORTH_DEBUG_ENABLED
// When debugging is enabled, we have runtime on/off control
extern bool debug_enabled;
void debug_raw(const char* fmt, ...);

// Macro that checks runtime flag and calls debug_raw with automatic newline
#define debug(fmt, ...)              \
  do {                               \
    if (debug_enabled) {             \
      debug_raw(fmt, ##__VA_ARGS__); \
    }                                \
  } while (0)
#else
// When debugging is disabled at compile time, debug() becomes a complete no-op
#define debug(fmt, ...) ((void)0)
#endif

// Core debug interface - always available regardless of compile-time flags
void debug_init(void);
void debug_on(void);
void debug_off(void);
bool debug_is_enabled(void);

#ifdef FORTH_DEBUG_ENABLED
// Enhanced debug macro that can handle buffer creation
#define debug_with_buffer(buffer_name, size, setup_code, fmt, ...) \
  do {                                                             \
    if (debug_enabled) {                                           \
      char buffer_name[size];                                      \
      setup_code debug_raw(fmt, ##__VA_ARGS__);                    \
    }                                                              \
  } while (0)
#else
#define debug_with_buffer(buffer_name, size, setup_code, fmt, ...) ((void)0)
#endif

void f_debug_on(word_t* self);
void f_debug_off(word_t* self);

#endif  // FORTH_DEBUG_H