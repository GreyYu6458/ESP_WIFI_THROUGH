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
#include "uart_init.h"

// queue
#include "queue.h"

// self header
#include "conform.h"
#include "conform_event.h"

/* 链接的密钥 */
extern const int32_t confirm_key;

extern const char* TAG;

/* 分配的端口 */
extern int32_t assigned_port;

/* 服务器地址，在第一次被扫描时初始化 */
extern struct sockaddr_in serviceAddr;

const char* TAG;
typedef struct
{
    size_t len;
    char* data;
}inner_data;



#define DEBUG


#endif