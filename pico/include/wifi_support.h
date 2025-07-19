#ifndef WIFI_SUPPORT_H
#define WIFI_SUPPORT_H

#include <stdbool.h>

// WiFi management functions
bool wifi_init(void);
bool wifi_connect(const char* ssid, const char* password);
bool wifi_disconnect(void);
bool wifi_is_connected(void);
void wifi_deinit(void);
void wifi_status(void);

#endif  // WIFI_SUPPORT_H