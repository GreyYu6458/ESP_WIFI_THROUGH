#ifndef _UDP_INIT_H_
#define _UDP_INIT_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "memory_pool.h"
#include "socket_init.h"
#include "msg_def.h"
#include "task.h"
#include "queue.h"
#include "config.h"
#include "esp_log.h"


typedef struct xUDPTask_t
{
    int hsocket;
    struct sockaddr_in to;
    
    char rec_task_name[15];
    char send_task_name[15];

    xQueueHandle udp_rec_queue;
    xQueueHandle udp_send_queue;
    xTaskHandle rec_task_handle;
    xTaskHandle send_task_handle;
    UBaseType_t uxPriority;
    void (*xUDPRecCallback)(struct xUDPTask_t *, void *, struct sockaddr_in *);

    xMemoryPoolHandle xMPool;
} xUDPTask_t;
typedef xUDPTask_t *xUDPTaskHandle;
typedef void (*xUDPRecCallBack_t)(xUDPTaskHandle, void *, struct sockaddr_in *);

extern xUDPTaskHandle xUDPTaskCreate(int sock, struct sockaddr *to, int16_t inQueueSize,
                                     int16_t outQueueSize, UBaseType_t uxPriority, xMemoryPoolHandle xMPool);

extern void xUDPTaskStart(xUDPTaskHandle xUDPTask);

extern void xUDPTaskSetRemote(xUDPTaskHandle xUDPTask, struct sockaddr_in *to);

extern void xUDPTaskSend(xUDPTaskHandle xUDPTask, const xMemoryBlockHandle xMB);

extern void xUDPTaskSetRecCallback(xUDPTaskHandle xUDPTask, xUDPRecCallBack_t xUDPRCb);

#endif