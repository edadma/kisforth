#include <stdio.h>

#include "pico/stdlib.h"
#include "repl.h"
#include "startup.h"
#include "stdlib_words.h"
#include "wifi_words.h"

int main() {
  stdio_init_all();

  while (!stdio_usb_connected()) {
    sleep_ms(100);
  }

  sleep_ms(500);
  printf("\n");

  forth_system_init();
  pico_register_wifi_words();
  pico_register_stdlib_words();
  print_startup_banner("Pico");

  repl();
  return 0;
}
