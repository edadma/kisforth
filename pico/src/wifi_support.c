#include "wifi_support.h"

#include <stdbool.h>
#include <stdio.h>

#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// WiFi state
static bool wifi_initialized = false;
static bool wifi_connected = false;

bool wifi_init(void) {
  if (wifi_initialized) {
    return true;
  }

  // Initialize CYW43 architecture
  if (cyw43_arch_init()) {
    printf("Failed to initialize CYW43 architecture\n");
    return false;
  }

  // Enable station mode
  cyw43_arch_enable_sta_mode();

  wifi_initialized = true;
  printf("WiFi initialized successfully\n");
  return true;
}

bool wifi_connect(const char* ssid, const char* password) {
  if (!wifi_initialized) {
    printf("WiFi not initialized\n");
    return false;
  }

  printf("Connecting to WiFi network: %s\n", ssid);

  // Attempt to connect
  int result = cyw43_arch_wifi_connect_timeout_ms(
      ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000);

  if (result == 0) {
    wifi_connected = true;
    printf("WiFi connected successfully\n");

    // Print IP address
    struct netif* netif = netif_default;
    if (netif && netif_is_up(netif)) {
      printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
    }

    return true;
  } else {
    printf("Failed to connect to WiFi (error %d)\n", result);
    return false;
  }
}

bool wifi_disconnect(void) {
  if (!wifi_initialized) {
    return false;
  }

  cyw43_arch_wifi_disconnect();
  wifi_connected = false;
  printf("WiFi disconnected\n");
  return true;
}

bool wifi_is_connected(void) {
  return wifi_connected &&
         (cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP);
}

void wifi_deinit(void) {
  if (!wifi_initialized) {
    return;
  }

  if (wifi_connected) {
    wifi_disconnect();
  }

  cyw43_arch_deinit();
  wifi_initialized = false;
  printf("WiFi deinitialized\n");
}

void wifi_status(void) {
  if (!wifi_initialized) {
    printf("WiFi: Not initialized\n");
    return;
  }

  printf("WiFi Status:\n");
  printf("  Initialized: %s\n", wifi_initialized ? "Yes" : "No");
  printf("  Connected: %s\n", wifi_is_connected() ? "Yes" : "No");

  if (wifi_is_connected()) {
    struct netif* netif = netif_default;
    if (netif && netif_is_up(netif)) {
      printf("  IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
      printf("  Gateway: %s\n", ip4addr_ntoa(netif_ip4_gw(netif)));
      printf("  Netmask: %s\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
    }
  }
}