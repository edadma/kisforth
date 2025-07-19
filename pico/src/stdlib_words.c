#include "stdlib_words.h"

#include <stdio.h>

#include "dictionary.h"
#include "error.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "stack.h"

// GPIO-INIT ( pin -- )
// Initialize a GPIO pin
static void f_gpio_init(word_t* self) {
  (void)self;

  cell_t pin = data_pop(ctx);

  if (pin < 0 || pin > 29) {  // RP2040 has GPIO 0-29
    error("GPIO-INIT: invalid pin number");
    return;
  }

  gpio_init((uint)pin);
}

// GPIO-OUT ( pin -- )
// Set GPIO pin as output
static void f_gpio_out(word_t* self) {
  (void)self;

  cell_t pin = data_pop(ctx);

  if (pin < 0 || pin > 29) {
    error("GPIO-OUT: invalid pin number");
    return;
  }

  gpio_set_dir((uint)pin, GPIO_OUT);
}

// GPIO-IN ( pin -- )
// Set GPIO pin as input
static void f_gpio_in(word_t* self) {
  (void)self;

  cell_t pin = data_pop(ctx);

  if (pin < 0 || pin > 29) {
    error("GPIO-IN: invalid pin number");
    return;
  }

  gpio_set_dir((uint)pin, GPIO_IN);
}

// GPIO-PUT ( pin value -- )
// Set GPIO pin high (non-zero) or low (zero)
static void f_gpio_put(word_t* self) {
  (void)self;

  cell_t value = data_pop(ctx);
  cell_t pin = data_pop(ctx);

  if (pin < 0 || pin > 29) {
    error("GPIO-PUT: invalid pin number");
    return;
  }

  gpio_put((uint)pin, value != 0);  // Convert to boolean
}

// GPIO-GET ( pin -- value )
// Read GPIO pin state (0 or 1)
static void f_gpio_get(word_t* self) {
  (void)self;

  cell_t pin = data_pop(ctx);

  if (pin < 0 || pin > 29) {
    error("GPIO-GET: invalid pin number");
    return;
  }

  bool state = gpio_get((uint)pin);
  data_push(ctx, state ? 1 : 0);
}

// MS ( u -- )
// Sleep for u milliseconds
static void f_ms(word_t* self) {
  (void)self;

  cell_t ms = data_pop(ctx);

  if (ms < 0) {
    error("MS: negative delay not allowed");
    return;
  }

  sleep_ms((uint32_t)ms);
}

// US ( u -- )
// Sleep for u microseconds
static void f_us(word_t* self) {
  (void)self;

  cell_t us = data_pop(ctx);

  if (us < 0) {
    error("US: negative delay not allowed");
    return;
  }

  sleep_us((uint64_t)us);
}

// TICKS ( -- u )
// Get milliseconds since boot
static void f_ticks(word_t* self) {
  (void)self;

  absolute_time_t now = get_absolute_time();
  uint32_t ms = to_ms_since_boot(now);

  data_push(ctx, (cell_t)ms);
}

// Register all Pico stdlib words
void pico_register_stdlib_words(void) {
  // GPIO words
  create_primitive_word("GPIO-INIT", f_gpio_init);
  create_primitive_word("GPIO-OUT", f_gpio_out);
  create_primitive_word("GPIO-IN", f_gpio_in);
  create_primitive_word("GPIO-PUT", f_gpio_put);
  create_primitive_word("GPIO-GET", f_gpio_get);

  // Timing words
  create_primitive_word("MS", f_ms);
  create_primitive_word("US", f_us);
  create_primitive_word("TICKS", f_ticks);

  printf("Pico stdlib words registered\n");
}