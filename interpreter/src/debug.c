#include "debug.h"

#include <stdarg.h>
#include <stdio.h>

#include "context.h"
#include "types.h"

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

#endif  // FORTH_DEBUG_ENABLED

// Core debug interface functions - always available
// This ensures Forth words DEBUG-ON/DEBUG-OFF can always be linked

#ifdef FORTH_DEBUG_ENABLED
void debug_init(void) {
  debug_enabled = false;  // Start with debugging off
  // When debugging disabled at compile time, this is a no-op
}
#endif

#ifdef FORTH_DEBUG_ENABLED
void debug_on(void) {
  debug_enabled = true;
  printf("Debug output enabled\n");
  fflush(stdout);
}
#endif

#ifdef FORTH_DEBUG_ENABLED
void debug_off(void) {
  debug_enabled = false;
  printf("Debug output disabled\n");
  fflush(stdout);
  // When debugging disabled at compile time, this is a no-op
}
#endif

bool debug_is_enabled(void) {
#ifdef FORTH_DEBUG_ENABLED
  return debug_enabled;
#else
  return false;  // Always disabled when not compiled in
#endif
}

// Debug control primitives

#ifdef FORTH_DEBUG_ENABLED
// DEBUG-ON ( -- ) Enable debug output
void f_debug_on(context_t* ctx, word_t* self) {
  (void)self;
  (void)ctx;

  debug_on();
}

// DEBUG-OFF ( -- ) Disable debug output
void f_debug_off(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  debug_off();
}
#endif
