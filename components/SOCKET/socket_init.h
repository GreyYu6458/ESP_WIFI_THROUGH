#ifndef _SOCKET_INIT_H_
#define _SOCKET_INIT_H_
// lwip api
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* 套接字初始化 返回套接字句柄 */
extern int socket_init(uint16_t port);

#endif