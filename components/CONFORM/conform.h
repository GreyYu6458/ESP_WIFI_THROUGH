#ifndef _CONFORM_H_
#define _CONFORM_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "msg_def.h"
#include "udp_task.h"
#include "self_detail_json.h"
#include "config.h"
#include "memory_pool.h"


typedef struct AuthenticateTask_t
{
    xUDPTaskHandle udp_adapter;
}xAuthenticateTask_t;
typedef xAuthenticateTask_t* xAuthenticateTaskHandle;

/*
* @brief            服务器验证类
* @param adapter    目标设备地址信息
*/
extern xAuthenticateTaskHandle xAuthenticateTaskCreate(xUDPTaskHandle adapter);

extern void xAuthenticateTaskStart(xAuthenticateTaskHandle xAuthenticateTask);

#endif