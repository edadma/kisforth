#include "wifi_words.h"

#include <stdio.h>

#include "boards/pico_w.h"  // For CYW43_WL_GPIO_LED_PIN
#include "dictionary.h"
#include "error.h"
#include "memory.h"
#include "pico/cyw43_arch.h"
#include "stack.h"
#include "text.h"
#include "wifi_support.h"

// WIFI-INIT ( -- flag )
// Initialize WiFi subsystem, return true if successful
static void f_wifi_init(word_t* self) {
  (void)self;

  bool success = wifi_init();
  data_push(success ? -1 : 0);  // Forth true/false convention
}

// WIFI-STATUS ( -- )
// Print detailed WiFi status information
static void f_wifi_status(word_t* self) {
  (void)self;

  wifi_status();
}

// WIFI-CONNECTED? ( -- flag )
// Check if WiFi is currently connected
static void f_wifi_connected_q(word_t* self) {
  (void)self;

  bool connected = wifi_is_connected();
  data_push(connected ? -1 : 0);  // Forth true/false convention
}

// WIFI-DISCONNECT ( -- flag )
// Disconnect from WiFi, return true if successful
static void f_wifi_disconnect(word_t* self) {
  (void)self;

  bool success = wifi_disconnect();
  data_push(success ? -1 : 0);  // Forth true/false convention
}

// LED-ON ( -- )
// Turn on the onboard LED (requires WiFi init)
static void f_led_on(word_t* self) {
  (void)self;

  // Set the onboard LED (GPIO 0 on CYW43 chip)
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
}

// LED-OFF ( -- )
// Turn off the onboard LED (requires WiFi init)
static void f_led_off(word_t* self) {
  (void)self;

  // Clear the onboard LED (GPIO 0 on CYW43 chip)
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

// LED-TOGGLE ( -- )
// Toggle the onboard LED state
static void f_led_toggle(word_t* self) {
  (void)self;

  // Read current state and toggle it
  bool current_state = cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !current_state);
}

// WIFI-CONNECT ( c-addr1 u1 c-addr2 u2 -- flag )
// Connect to WiFi network with SSID and password
// c-addr1 u1 = SSID string
// c-addr2 u2 = password string
static void f_wifi_connect(word_t* self) {
  (void)self;

  // Get password string (top of stack)
  cell_t pass_len = data_pop();
  forth_addr_t pass_addr = (forth_addr_t)data_pop();

  // Get SSID string
  cell_t ssid_len = data_pop();
  forth_addr_t ssid_addr = (forth_addr_t)data_pop();

  // Validate string lengths
  if (ssid_len <= 0 || ssid_len > 32 || pass_len < 0 || pass_len > 63) {
    error(ctx, "Invalid SSID or password length");
    return;
  }

  // Copy strings to null-terminated buffers
  char ssid[33];      // Max SSID length + null terminator
  char password[64];  // Max WPA password length + null terminator

  // Copy SSID
  for (int i = 0; i < ssid_len; i++) {
    ssid[i] = forth_c_fetch(ssid_addr + i);
  }
  ssid[ssid_len] = '\0';

  // Copy password
  for (int i = 0; i < pass_len; i++) {
    password[i] = forth_c_fetch(pass_addr + i);
  }
  password[pass_len] = '\0';

  // Attempt connection
  bool success = wifi_connect(ssid, password);
  data_push(success ? -1 : 0);  // Forth true/false convention
}

// Register all WiFi words for Pico platform
void pico_register_wifi_words(void) {
  create_primitive_word("WIFI-INIT", f_wifi_init);
  create_primitive_word("WIFI-STATUS", f_wifi_status);
  create_primitive_word("WIFI-CONNECTED?", f_wifi_connected_q);
  create_primitive_word("WIFI-DISCONNECT", f_wifi_disconnect);
  create_primitive_word("WIFI-CONNECT", f_wifi_connect);

  // LED control words (require WiFi init)
  create_primitive_word("LED-ON", f_led_on);
  create_primitive_word("LED-OFF", f_led_off);
  create_primitive_word("LED-TOGGLE", f_led_toggle);

  printf("WiFi and LED words registered\n");
}