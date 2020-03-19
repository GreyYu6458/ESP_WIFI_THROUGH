#ifndef _MAIN_H_
#define _MAIN_H_
// clib
#include <string.h>

// freertos api
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// esp api
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"

// chip api
#include "nvs.h"
#include "nvs_flash.h"

// lwip api
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

// uart
#include "uart.h"

const char* TAG;

#define DEBUG


#endif