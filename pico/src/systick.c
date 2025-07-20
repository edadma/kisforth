#include "systick.h"

#include <stdio.h>

#include "dictionary.h"
#include "error.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include "stack.h"

// Global state for SysTick timer
static context_t systick_context;
static cell_t handler_xt = 0;     // Execution token of handler word
static uint32_t interval_ms = 0;  // Timer interval in milliseconds
static repeating_timer_t timer;   // Pico SDK timer structure
static bool timer_running = false;

// Timer callback function - called from interrupt context
static bool systick_callback(repeating_timer_t* rt) {
  (void)rt;  // Unused parameter

  // Only execute if we have a valid handler
  if (handler_xt != 0) {
    // Find the word in the dictionary
    word_t* handler_word = (word_t*)handler_xt;

    // Execute the handler in the SysTick context
    if (handler_word && handler_word->cfunc) {
      handler_word->cfunc(&systick_context, handler_word);
    }
  }

  return true;  // Keep repeating
}

// Initialize SysTick system
void systick_init(void) {
  // Initialize the dedicated SysTick context
  context_init(&systick_context, "SYSTICK_IRQ", true);

  // Initialize global state
  handler_xt = 0;
  interval_ms = 0;
  timer_running = false;

  printf("SysTick timer system initialized\n");
}

// SYSTICK-START ( interval-ms xt -- )
// Start SysTick timer with given interval and handler
static void f_systick_start(context_t* ctx, word_t* self) {
  (void)self;

  // Pop arguments from data stack
  cell_t xt = data_pop(ctx);
  cell_t interval = data_pop(ctx);

  // Validate interval (1ms to 1000ms)
  if (interval < 1 || interval > 1000) {
    printf("SYSTICK-START: interval must be 1-1000ms\n");
    return;
  }

  // Stop any existing timer
  if (timer_running) {
    cancel_repeating_timer(&timer);
    timer_running = false;
  }

  // Store new settings
  handler_xt = xt;
  interval_ms = (uint32_t)interval;

  // Start the timer
  if (add_repeating_timer_ms(-(int32_t)interval_ms, systick_callback, NULL,
                             &timer)) {
    timer_running = true;
    printf("SysTick started: %lu ms interval\n", interval_ms);
  } else {
    printf("SysTick failed to start\n");
    handler_xt = 0;
    interval_ms = 0;
  }
}

// SYSTICK-STOP ( -- )
// Stop SysTick timer
static void f_systick_stop(context_t* ctx, word_t* self) {
  (void)ctx;
  (void)self;

  if (timer_running) {
    cancel_repeating_timer(&timer);
    timer_running = false;
    handler_xt = 0;
    interval_ms = 0;
    printf("SysTick stopped\n");
  } else {
    printf("SysTick not running\n");
  }
}

// Register SysTick words
void pico_register_systick_words(void) {
  create_primitive_word("SYSTICK-START", f_systick_start);
  create_primitive_word("SYSTICK-STOP", f_systick_stop);

  printf("SysTick words registered\n");
}