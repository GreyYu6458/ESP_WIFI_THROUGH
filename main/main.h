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

//cjson
#include "cJSON.h"

// uart
#include "uart_task.h"
#include "udp_task.h"

// queue
#include "queue.h"

// self header
#include "config.h"
#include "self_detail_json.h"
#include "socket_init.h"
#include "udp_task.h"
#include "conform.h"

/* 分配的端口 */
extern int32_t assigned_port;

/* 服务器地址，在第一次被扫描时初始化 */
extern struct sockaddr_in serviceAddr;


#define DEBUG


#endif