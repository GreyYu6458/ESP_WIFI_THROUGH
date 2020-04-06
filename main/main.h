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

// self header
#include "config.h"
#include "uart_task.h"
#include "udp_task.h"
#include "self_detail_json.h"
#include "bridge.h"
#include "uart_init.h"
#include "socket_init.h"
#include "memory_pool.h"
#include "conform.h"


#define DEBUG


#endif