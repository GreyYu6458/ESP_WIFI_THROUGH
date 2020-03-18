#ifndef _LINK_WIFI_H_
#define _LINK_WIFI_H_
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "main.h"

#define WIFI_SSID               "Xiaomi_56A8"
#define WIFI_PASS               "1sky-shanghai"
#define MAXIMUM_RETRY           5

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

extern void wifi_init_sta(void);

extern tcpip_adapter_ip_info_t self_ip;

#endif